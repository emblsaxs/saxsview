/*
 * Copyright (C) 2012 Daniel Franke <dfranke@users.sourceforge.net>
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

#include "columns.h"
#include "saxsdocument.h"
#include "saxsdocument_format.h"

#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <math.h>

/**************************************************************************/
static int
malvern_txt_parse_header(struct saxs_document *doc,
                         struct line *firstline, struct line *lastline) {

  /* TODO: Anything useful here? Ignore this for now. */

  return 0;
}

static int
malvern_txt_parse_data(struct saxs_document *doc,
                       struct line *firstline, struct line *lastline) {

  struct saxs_curve *cri, *cuv, *crals, *cmw, *cpeak;
  double ml, ri, uv, rals, mw, ign;

  /* Skip the column names. */
  if (firstline)
    firstline = firstline->next;

  /*
   * We are interested in: RI (2), UV (3), RALS (4) and Molecular Weight (7);
   * NOTE: the column numbers in parantheses are '1'-based.
   *
   * Further we need a peak indicator, '0' if no peak (only 4 output columns
   * in the data file) and '1' on peak (13 output columns).
   *
   * Unfortunately, saxs_reader_columns_parse() can not deal with missing
   * values, a feature we'd need here. However, since we want to generate
   * an artificial peak indicator anyway, we deal with it here.
   */
  cri   = saxs_document_add_curve(doc, "RI", SAXS_CURVE_EXPERIMENTAL_SCATTERING_DATA);
  cuv   = saxs_document_add_curve(doc, "UV", SAXS_CURVE_EXPERIMENTAL_SCATTERING_DATA);
  crals = saxs_document_add_curve(doc, "RALS", SAXS_CURVE_EXPERIMENTAL_SCATTERING_DATA);
  cmw   = saxs_document_add_curve(doc, "Molecular Weight (kDa)", SAXS_CURVE_EXPERIMENTAL_SCATTERING_DATA);
  cpeak = saxs_document_add_curve(doc, "Peak Indicator", SAXS_CURVE_EXPERIMENTAL_SCATTERING_DATA);

  while (firstline != lastline) {
    if (sscanf(firstline->line_buffer,
               "%lf %lf %lf %lf",
               &ml, &ri, &uv, &rals) == 4) {

      saxs_curve_add_data(cri,   ml, 0.0, ri,   0.0);
      saxs_curve_add_data(cuv,   ml, 0.0, uv,   0.0);
      saxs_curve_add_data(crals, ml, 0.0, rals, 0.0);
      saxs_curve_add_data(cmw,   ml, 0.0, mw,   0.0);
      saxs_curve_add_data(cpeak, ml, 0.0, 0.0,  0.0);

    } else if (sscanf(firstline->line_buffer,
                      "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
                      &ml, &ri, &uv, &rals, &ign, &ign, &mw, &ign, &ign,
                      &ign, &ign, &ign, &ign, &ign) == 13) {

      saxs_curve_add_data(cri,   ml, 0.0, ri,   0.0);
      saxs_curve_add_data(cuv,   ml, 0.0, uv,   0.0);
      saxs_curve_add_data(crals, ml, 0.0, rals, 0.0);
      saxs_curve_add_data(cmw,   ml, 0.0, pow(10, mw) / 1000.0, 0.0);
      saxs_curve_add_data(cpeak, ml, 0.0, 1.0,  0.0);
    }

    firstline = firstline->next;
  }

  return 0;
}

int
malvern_txt_read(struct saxs_document *doc,
                 struct line *firstline, struct line *lastline) {

  struct line *header, *data, *footer;

  /*
   * The header starts at the first line and ends when the data begins with:
   *     "Ret. Vol.\tRI\tUV\tRALS\tAdjusted RI [...]"
   */
  header = firstline;
  data   = header;
  while (data && !(strstr(data->line_buffer, "Ret. Vol.")
                    && strstr(data->line_buffer, "RI")
                    && strstr(data->line_buffer, "UV")
                    && strstr(data->line_buffer, "RALS")
                    && strstr(data->line_buffer, "Adjusted RI")
                    && strstr(data->line_buffer, "Adjusted RALS")
                    && strstr(data->line_buffer, "Molecular Weight")
                    && strstr(data->line_buffer, "Cumulative Weight Fraction")
                    && strstr(data->line_buffer, "Normalized Wt Fr")
                    && strstr(data->line_buffer, "Normalized Mole Fraction")
                    && strstr(data->line_buffer, "Conc.")
                    && strstr(data->line_buffer, "WF/dLogM")))
    data = data->next;

  /* There is no spoon, sorry, footer. */
  footer = lastline;

  /*
   * If the beginning of the data was not found,
   * this is not a malvern file.
   */
  if (!data)
    return ENOTSUP;

  malvern_txt_parse_header(doc, header, data);
  malvern_txt_parse_data(doc, data, footer);

  return 0;
}


/**************************************************************************/
void
saxs_document_format_register_malvern_txt() {
  saxs_document_format malvern_txt = {
     "txt", "malvern-txt",
     "Data from Malvern OmniSEC text files.",
     malvern_txt_read, NULL, NULL
  };

  saxs_document_format_register(&malvern_txt);
}
