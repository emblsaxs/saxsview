/*
 * Read files in cansas-XML format as defined by the cansas working group.
 * See also:
 *   http://www.smallangles.net/wgwiki/index.php/cansas1d_documentation
 *
 * Copyright (C) 2009, 2010 Daniel Franke <dfranke@users.sourceforge.net>
 *
 * This file is part of libsaxsdocument.
 *
 * libsaxsdocument is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General
 * Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any
 * later version.
 *
 * libsaxsdocument is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with libsaxsdocument. If not,
 * see <http://www.gnu.org/licenses/>.
 */

#include "saxsdocument.h"
#include "saxsdocument_format.h"
#include "columns.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <libxml/xmlreader.h>

static int
cansas_read_callback(struct line **line, char *buffer, int len) {
  /*
   * The line reader strips off the '\n', here we need to add them
   * again as otherwise the XML parser will see run together lines
   * and possibly error out.
   */
  if (*line) {
    snprintf(buffer, len, "%s\n", (*line)->line_buffer);

    *line = (*line)->next;
    return strlen(buffer);

  } else
    return 0;
}

/**************************************************************************/
/*
 * Node names are based on r32 of
 *   http://svn.smallangles.net/trac/canSAS/browser/1dwg/trunk/cansas1d.xsd
 */
static void cansas_xml_1_0_process_node(saxs_document *doc, xmlTextReaderPtr reader) {
  static xmlChar *text;
  static saxs_curve *curve = NULL;
  static double x = 0.0, dx = 0.0, y = 0.0, dy = 0.0;

  xmlChar *name;

  switch (xmlTextReaderNodeType(reader)) {
    case XML_READER_TYPE_ELEMENT:
      name = xmlTextReaderLocalName(reader);
      if (xmlStrEqual(name, BAD_CAST("SASdata"))) {
        xmlChar *title = xmlTextReaderGetAttribute(reader, BAD_CAST("name"));
        curve = saxs_document_add_curve(doc, (const char*)title,
                                        SAXS_CURVE_EXPERIMENTAL_SCATTERING_DATA);
        if (title)
          xmlFree(title);

      } else if (xmlStrEqual(name, BAD_CAST("Idata"))) {
        x = dx = y = dy = 0.0;
      }
      xmlFree(name);
      break;

    case XML_READER_TYPE_END_ELEMENT:
      name = xmlTextReaderLocalName(reader);
      if (xmlStrEqual(name, BAD_CAST("Q"))) {
        x = strtod((const char *)text, NULL);

      } else if (xmlStrEqual(name, BAD_CAST("Qdev"))) {
        dx = strtod((const char *)text, NULL);

      } else if (xmlStrEqual(name, BAD_CAST("I"))) {
        y = strtod((const char *)text, NULL);

      } else if (xmlStrEqual(name, BAD_CAST("Idev"))) {
        dy = strtod((const char *)text, NULL);

      } else if (xmlStrEqual(name, BAD_CAST("Idata"))) {
        saxs_curve_add_data(curve, x, dx, y, dy);
      }
      xmlFree(name);
      break;

    case XML_READER_TYPE_TEXT:
      if (text)
        xmlFree(text);
      text = xmlTextReaderValue(reader);
      break;
  }
}

int cansas_xml_1_0_read(saxs_document *doc, struct line *firstline, struct line *lastline) {

  /*
   * Work around an problem in cansas-1.0 and libxml2:
   * The cansas-standard v1.0 uses a deprecated (relative) format of the
   * URI used to define the default namespace [1], while the stream-reader
   * interface of libxml2 wrongly errors out on exact these namespace
   * definitions [2].
   *
   * Work-around: The DOM-api is not affected, thus read the full document
   * from lines first, then walk it.
   *
   * [1] http://svn.smallangles.net/trac/canSAS/ticket/20
   * [2] http://mail.gnome.org/archives/xml/2009-September/msg00072.html
   */
  xmlTextReaderPtr reader;

  xmlDocPtr xmldoc = xmlReadIO((xmlInputReadCallback)cansas_read_callback,
                               NULL, &firstline, NULL, NULL,
                               XML_PARSE_NOWARNING | XML_PARSE_NOERROR);
  if (!xmldoc)
    return EINVAL;

  reader = xmlReaderWalker(xmldoc);
  if (!reader)
    return EINVAL;

  /* Check the first node that this is the right document version. */
  while (xmlTextReaderRead(reader) == 1) {
    /* only check the very first element */
    if (xmlTextReaderNodeType(reader) == XML_READER_TYPE_ELEMENT
        && xmlTextReaderDepth(reader) == 0) {

      xmlChar *name, *attr;

      name = xmlTextReaderLocalName(reader);
      if (!xmlStrEqual(name, BAD_CAST("SASroot"))) {
        xmlFree(name);
        break;
      }
      xmlFree(name);

      attr = xmlTextReaderGetAttribute(reader, BAD_CAST("version"));
      if (!xmlStrEqual(attr, BAD_CAST("1.0"))) {
        xmlFree(attr);
        xmlFreeTextReader(reader);
        xmlFreeDoc(xmldoc);

        return ENOTSUP;
      }
      xmlFree(attr);

      break;
    }
  }

  /* Ok, now read all the nodes. */
  while (xmlTextReaderRead(reader) == 1)
    cansas_xml_1_0_process_node(doc, reader);

  xmlFreeTextReader(reader);
  xmlFreeDoc(xmldoc);

  return 0;
}

/**************************************************************************/
void
saxs_document_format_register_cansas_xml() {
  saxs_document_format cansas_xml = {
     "xml", "cansas-xml-v1.0", "CANSAS Working Group XML v1.0",
     cansas_xml_1_0_read, NULL, NULL
  };

  saxs_document_format_register(&cansas_xml);
}
