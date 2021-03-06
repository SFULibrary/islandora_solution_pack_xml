<?php

/**
 * @file
 * Implementation of classes from the batch ingest framework.
 */

/**
 * Batch object class.
 */
class IslandoraSimpleXmlBatchObject extends IslandoraBatchObject {
  protected $xmlObjFilePath;
  protected $preprocessorParameters;

  /**
   * Constructor.
   */
  public function __construct(IslandoraTuque $connection, string $xml_file_path, $batch_parameters) {
    parent::__construct(NULL, $connection->repository);

    $this->xmlObjFilePath = $xml_file_path;
    $this->preprocessorParameters = $batch_parameters;
  }

  /**
   * Assemble the base datastreams.
   */
  public function batchProcess() {
    // OBJ datastream.
    $obj = $this->constructDatastream('OBJ', 'M');
    $obj->label = 'OBJ Datastream';
    $obj->mimetype = 'application/xml';
    $obj->setContentFromFile($this->xmlObjFilePath, FALSE);
    $this->ingestDatastream($obj);

    // MODS, then from it DC, datastreams.
    if ($mods = $this->getMods()) {
      $this->ingestDatastream($mods);

      module_load_include('inc', 'islandora_solution_pack_xml', 'includes/utilities');
      $path_to_mods_to_dc_xsl = drupal_get_path('module', 'islandora_batch') . '/transforms/mods_to_dc.xsl';
      $mods_to_dc_xsl = file_get_contents($path_to_mods_to_dc_xsl);
      $dc_content = islandora_solution_pack_xml_apply_xslt($mods_to_dc_xsl, $mods->content);
      $dc = $this->constructDatastream('DC', 'M');
      $dc->mimetype = 'application/xml';
      $dc->label = 'DC Record';
      $dc->setContentFromString($dc_content);
      $this->ingestDatastream($dc);

      // Get object label from dc:title.
      $dom = new DOMDocument();
      $dom->loadXML($dc_content);
      $xpath = new DOMXPath($dom);
      $xpath->registerNamespace('dc', 'http://purl.org/dc/elements/1.1/');
      $titles = $xpath->query("//dc:title");
      if ($titles->length > 0) {
        $this->label = $titles->item(0)->nodeValue;
      }
      else {
        $this->label = '[No object label available]';
      }
    }

    // TN datastream.
    if ($tn = $this->getTn()) {
      $this->ingestDatastream($tn);
    }

    // RENDER_STYLESHEET datastream.
    if ($rs = $this->getRenderStylesheet()) {
      $this->ingestDatastream($rs);
    }

    // Additional datastreams.
    $additional_datastreams = $this->getAdditionalDatastreams();
    if (count($additional_datastreams)) {
      foreach ($additional_datastreams as $additional) {
        $this->ingestDatastream($additional);
      }
    }

    $this->addRelationships();

    // Indicate that this object is ready to be ingested.
    return ISLANDORA_BATCH_STATE__DONE;
  }

  /**
   * Add relationships.
   */
  public function addRelationships() {
    $this->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOfCollection', $this->preprocessorParameters['parent']);
    $this->models = 'islandora:sp_simple_xml';
  }

  /**
   * Get a list of resources.
   */
  public function getResources() {
    return array();
  }

  /**
   * Creates a TN datastream from a file matching the base object.
   *
   * @return object|FALSE
   *   A datastream object for the thumbnail, or FALSE if no datastream
   *   was created.
   */
  private function getTn() {
    $tn_extensions = array('png', 'jpg', 'jpeg', 'gif');
    $pathinfo = pathinfo($this->xmlObjFilePath);
    $dir_name = $pathinfo['dirname'];
    $file_name = $pathinfo['filename'];
    $tn_file_path = '';
    foreach ($tn_extensions as $ext) {
      $tn_file_path = $dir_name . DIRECTORY_SEPARATOR . $file_name . '.TN.' . $ext;
      if (file_exists($tn_file_path)) {
        break;
      }
    }

    if (file_exists($tn_file_path)) {
      $tn_mime_detector = new MimeDetect();
      $tn = $this->constructDatastream('TN', 'M');
      $tn->mimetype = $tn_mime_detector->getMimetype($tn_file_path);
      $tn->label = 'Thumbnail';
      $tn->setContentFromFile($tn_file_path, FALSE);
      return $tn;
    }
    else {
      $default_tn_path = variable_get('islandora_solution_pack_xml_default_thumbnail_path', '');
      if (strlen($default_tn_path) && file_exists($default_tn_path)) {
        $tn_mime_detector = new MimeDetect();
        $tn = $this->constructDatastream('TN', 'M');
        $tn->mimetype = $tn_mime_detector->getMimetype($default_tn_path);
        $tn->label = 'Thumbnail';
        $tn->setContentFromFile($default_tn_path, FALSE);
        return $tn;
      }
    }
    return FALSE;
  }

  /**
   * Creates a MODS datastream.
   *
   * If a file in the same directory as the OBJ file and matching its basename
   * plus '.MODS.xml' exists, populates the MODS datastream with that file;
   * if no such file exists, generates a minimal, templated MODS datastream.
   *
   * @return obj|FALSE
   *   A datastream object for the MODS document, or FALSE if no datastream
   *   was created.
   */
  private function getMods() {
    $pathinfo = pathinfo($this->xmlObjFilePath);
    $dir_name = $pathinfo['dirname'];
    $file_name = $pathinfo['filename'];
    $mods_file_path = $dir_name . DIRECTORY_SEPARATOR . $file_name . '.MODS.xml';
    if (file_exists($mods_file_path)) {
      $mods = $this->constructDatastream('MODS', 'M');
      $mods->mimetype = 'application/xml';
      $mods->label = 'MODS metadata';
      $mods->setContentFromFile($mods_file_path, FALSE);
      return $mods;
    }
    else {
      // In the absence of a poper MODS file, generate a minimal one.
      $mods = $this->constructDatastream('MODS', 'M');
      $mods->content = <<<EOQ
<mods xmlns='http://www.loc.gov/mods/v3'>
  <titleInfo>
    <title>{$file_name}</title>
  </titleInfo>
</mods>
EOQ;
      $mods->mimetype = 'application/xml';
      $mods->label = 'MODS metadata';
      return $mods;
    }
    return FALSE;
  }

  /**
   * Creates a RENDER_STYLESHEET datastream.
   *
   * @return object|FALSE
   *   A datastream object for the render stylesheet, or FALSE if no datastream
   *   was created.
   */
  private function getRenderStylesheet() {
    $pathinfo = pathinfo($this->xmlObjFilePath);
    $dir_name = $pathinfo['dirname'];
    $file_name = $pathinfo['filename'];
    $rs_file_path = $dir_name . DIRECTORY_SEPARATOR . $file_name . '.xsl';
    if (file_exists($rs_file_path)) {
      $rs = $this->constructDatastream('RENDER_STYLESHEET', 'M');
      $rs->mimetype = 'application/xml';
      $rs->label = 'Render stylesheet';
      $rs->setContentFromFile($rs_file_path, FALSE);
      return $rs;
    }
    else {
      return FALSE;
    }
  }

  /**
   * Creates any additional datastreams for the object based on filename.
   *
   * For a base XML file with the name somefile.xml, a datastream with the
   * DSID 'FOO' will be created from a file with the name somfile.FOO.ext.
   * The datastream's mimetype will be derived from .ext. Its label will
   * be 'FOO datastream'.
   *
   * @return array
   *   A list of additional datastreams to ingest with the current object.
   */
  private function getAdditionalDatastreams() {
    $datastreams = array();
    $pathinfo = pathinfo($this->xmlObjFilePath);
    $dir_name = $pathinfo['dirname'];
    $file_name = $pathinfo['filename'];
    $base_path = $dir_name . DIRECTORY_SEPARATOR . $file_name;

    $ds_filepaths = glob($base_path . '*');
    foreach ($ds_filepaths as $ds_filepath) {
      // We don't want the base .xml file.
      if ($ds_filepath != $base_path . '.xml') {
        // Or the MODS, TN, or RENDER_STYLESHEET files.
        if (!preg_match('/\.(TN|MODS|RENDER_STYLESHEET)/', $ds_filepath)) {
          $ds_filepath_filename = pathinfo($ds_filepath, PATHINFO_FILENAME);
          // The DSID is not an extension, but it will be the part after
          // the final . in $ds_filepath_filename.
          $dsid = pathinfo($ds_filepath_filename, PATHINFO_EXTENSION);
          if (strlen($dsid)) {
            $ds = $this->constructDatastream($dsid, 'M');
            $mime_detector = new MimeDetect();
            $ds->mimetype = $mime_detector->getMimetype($ds_filepath);
            $ds->label = $dsid . ' datastream';
            $ds->setContentFromFile($ds_filepath, FALSE);
            $datastreams[] = $ds;
          }
        }
      }
    }
    return $datastreams;
  }

}
