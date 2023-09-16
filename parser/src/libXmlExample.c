/**
 * section: Tree
 * synopsis: Navigates a tree to print element names
 * purpose: Parse a file to a tree, use xmlDocGetRootElement() to
 *          get the root element, then walk the document and print
 *          all the element name in document order.
 * usage: tree1 filename_or_URL
 * test: tree1 test2.xml > tree1.tmp && diff tree1.tmp $(srcdir)/tree1.res
 * author: Dodji Seketeli
 * copy: see Copyright for the status of this software.
 */
/*
Author: Michael Harris
Date: 02/10/2021
*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlschemastypes.h>
#include "GPXParser.h"
#include "LinkedListAPI.h"
#include "GPXHelpers.h"

/*
 *To compile this file using gcc you can type.  Alternatively, see the Makefile for include path settings
 *gcc `xml2-config --cflags --libs` -o xmlexample libxml2-example.c
 */


/**
 * Simple example to parse a file called "file.xml", 
 * walk down the DOM, and print the name of the 
 * xml elements nodes.
 */
int
main(int argc, char **argv)
{
    GPXdoc* doc = createValidGPXdoc(argv[1], argv[2]);

    validateGPXDoc(doc, argv[2]);

    writeGPXdoc(doc, "someName.gpx");

    deleteGPXdoc(doc);
    
    //printf("Dist = %d\f", isLoopTrack(getTrack(doc, "Reynolds To  Rozh. parking lot"), 2));

    return 0;
}