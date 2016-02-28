# Islandora XML Solution Pack

Simple Islandora Solution Pack that allows for ingesting and viewing XML files. This solution pack is a successor to the overly complex [Islandora Feeds](https://github.com/mjordan/islandora_feeds) module.

## Introduction

This module provides basic support for ingesting and viewing XML OBJ files in Islandora. The solution pack is "simple" because:

* It does not offer any way of editing the XML files. If users need to modify an XML file, they must replace the object's OBJ datastream using the standard tools provided within an object's Datastreams tab.
* It does not generate any derivatives.

Users may upload a thumbnail image for their XML object. Objects managed by this solution pack also have a MODS datastream just like other Islandora objects do.

## Rendering the OBJ datastream using XSL Transformations

The module allows the use of XSLT stylesheets in the following ways:

1. Users may upload a stylesheet when they create an object managed by this solution pack. The stylesheet becomes a datastream on the object with the datastream ID 'RENDER_STYLESHEET' and is applied to the XML OBJ file when users view the object. So in a sense this module creates its derivatives on demand, not on ingest.
2. Owners of collections may upload an XSL stylesheet as a datastream on a collection object. If this datastream has an ID of 'RENDER_STYLESHEET', it is used for all XML objects that are members of the collection (unless a member object has its own RENDER_STYLESHEET datastream).
3. If neither of the above is true, the XML file is simply escaped and rendered betweem HTML &lt;pre&gt; tags, or, if a viewer module (like the one included with this solution pack) is enabled and configured, it renders the XML output.

## Rendering the OBJ datastream using viewers

This solution pack supports Islandora viewer modules, and comes with a simple viewer module that if enabled and configured as the default viewer for XML objects allows easy styling of XML files using the [Google Javascript Prettifier](https://github.com/google/code-prettify).

Note that if this viewer is enabled, the XML OBJ datastream content is not styled with the RENDER_STYLESHEET XSLTs as described above. However, third-party viewers are free to use the RENDER_STYLESHEET or any other stylesheet they wish.

## Batch loading

Objects managed by this module cannot be loaded using Islandora Batch, but a simple custom Drush-based loader is available in the `modules` subdirectory.

## Requirements

* [Islandora](https://github.com/Islandora/islandora)

## Maintainer

* [Mark Jordan](https://github.com/mjordan)

## To do

* Add checks for well formedness on XML and XSLT files as they are uploaded.
* Add checks for validity against a specific schema or DTD (maybe one that is attached to the collection object as a datastream?).
* Write some additional viewers that present the XML content in interesting ways like [this](https://www.sencha.com/forum/showthread.php?163680-Implementing-treeview-using-xml-data) or [this](http://blog.ashwani.co.in/blog/2013-07-18/stylize-your-xml-with-jquery-xml-tree-plugin/).
* Provide Solr configs for allowing the indexing of XML element content for advanced searches.
* Provide sample XSLT stylesheets for common types of XML objects, like "flat" XML, TEI, EAD, etc.

## Development and feedback

Pull requests are welcome, as are use cases and suggestions.

## License

* [GPLv3](http://www.gnu.org/licenses/gpl-3.0.txt)