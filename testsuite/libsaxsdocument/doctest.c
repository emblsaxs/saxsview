/*
 * Testing for libsaxsdocument
 * Copyright (C) 2009 Daniel Franke <dfranke@users.sourceforge.net>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VERIFY(cond)                              \
  if (!(cond)) {                                  \
    fprintf(stderr, "%s:%d: check '%s' failed\n", \
             __FILE__, __LINE__, #cond);          \
    exit(EXIT_FAILURE);                           \
  }


/* move forward to next line */
static void skip_comment(FILE *fd) {
  while (!feof(fd))
    if (fgetc(fd) == '\n')
      break;
}

static void skip_ws(FILE *fd) {
  int c;
  while (!feof(fd)) {
    c = fgetc(fd);
    if (!isspace(c)) {
      ungetc(c, fd);
      break;
    }
  }
}

static char* read_field(FILE *fd) {
  char buf[1024] = { '\0' }, *bufptr = buf;

  skip_ws(fd);

  while (!feof(fd)) {
    int c = fgetc(fd);
    switch (c) {
      case '#':
        skip_comment(fd);
        break;

      case '\t':
        *bufptr++ = ' ';

      case '\r':
        break;

      case ';':
      case '\n':
        return strdup(buf);

      default:
        *bufptr++ = (char)c;
    }
  }

  return strdup(buf);
}


struct expect {
  char *exp_filename;
  int exp_curve_count;
  int exp_property_count;

  struct curve {
    char *exp_curve_title;
    int exp_curve_type;
    int exp_curve_data_count;
    struct curve *next;
  } *exp_curves;

  struct property {
    char *exp_property_name, *exp_property_value;
    struct property *next;
  } *exp_properties;
};

static struct expect* expect_get() {
  struct expect *exp = malloc(sizeof(struct expect));

  exp->exp_filename = NULL;
  exp->exp_curve_count = 0;
  exp->exp_property_count = 0;
  exp->exp_curves = NULL;
  exp->exp_properties = NULL;

  return exp;
}

static void expect_append_curve(struct expect *exp, char *title,
                                int type, int data_count) {

  struct curve *c = malloc(sizeof(struct curve));

  c->exp_curve_title = title;
  c->exp_curve_type = type;
  c->exp_curve_data_count = data_count;
  c->next = NULL;

  if (exp->exp_curves != NULL) {
    struct curve *head = exp->exp_curves;
    while (head && head->next)
      head = head->next;

    head->next = c;

  } else
    exp->exp_curves = c;
}

static void expect_append_property(struct expect *exp, char *name,
                                   char *value) {

  struct property *p = malloc(sizeof(struct property));

  p->exp_property_name = name;
  p->exp_property_value = value;
  p->next = NULL;

  if (exp->exp_properties != NULL) {
    struct property *head = exp->exp_properties;
    while (head && head->next)
      head = head->next;

    head->next = p;

  } else
    exp->exp_properties = p;
}



static void expect_read_document_fields(struct expect *exp, FILE *fd) {
  char *tmp;

  exp->exp_filename = read_field(fd);

  tmp = read_field(fd);
  VERIFY(sscanf(tmp, "%d", &exp->exp_curve_count) == 1);
  free(tmp);

  tmp = read_field(fd);
  VERIFY(sscanf(tmp, "%d", &exp->exp_property_count) == 1);
  free(tmp);
}

static void expect_read_curve_fields(struct expect *exp, FILE *fd) {
  char *tmp,  *title;
  int type, data_count;

  title = read_field(fd);

  tmp = read_field(fd);
  VERIFY(sscanf(tmp, "%d", &type) == 1);
  free(tmp);

  tmp = read_field(fd);
  VERIFY(sscanf(tmp, "%d", &data_count) == 1);
  free(tmp);

  expect_append_curve(exp, title, type, data_count);
}

static void expect_read_property_fields(struct expect *exp, FILE *fd) {
  char *name = read_field(fd);
  char *value = read_field(fd);

  expect_append_property(exp, name, value);
}


int expect_read(struct expect *exp, const char *filename) {
  char buffer[1024], *buf;

  FILE *fd = fopen(filename, "r");
  VERIFY(fd != NULL);

  while (!feof(fd)) {
    char *field = read_field(fd);

    if (strcmp(field, "document") == 0)
      expect_read_document_fields(exp, fd);

    else if (strcmp(field, "curve") == 0)
      expect_read_curve_fields(exp, fd);

    else if (strcmp(field, "property") == 0)
      expect_read_property_fields(exp, fd);

    else if (strcmp(field, "") == 0) {
      /* probably empty line or end of file, do nothing */
    } else
      VERIFY(!"parse error: unknown field type");

    free(field);
  }

  fclose(fd);
  return 0;
}


void expect_free(struct expect *exp) {
  if (exp->exp_filename)
    free(exp->exp_filename);

  struct curve *current_curve = exp->exp_curves, *prev_curve;
  while (current_curve) {
    prev_curve = current_curve;
    current_curve = current_curve->next;
    if (prev_curve->exp_curve_title)
      free(prev_curve->exp_curve_title);
    free(prev_curve);
  }

  struct property *current_property = exp->exp_properties, *prev_property;
  while (current_property) {
    prev_property = current_property;
    current_property = current_property->next;
    if (prev_property->exp_property_name)
      free(prev_property->exp_property_name);
    if (prev_property->exp_property_value)
      free(prev_property->exp_property_value);
    free(prev_property);
  }

  free(exp);
}




static int verify(saxs_document *doc, struct expect *exp) {
  struct curve *c;
  struct property *p;

  struct saxs_curve *sc;
  struct saxs_property *sp;
  int cnt;

  /* Filename not checked due to path components */
  VERIFY(strstr(saxs_document_filename(doc), exp->exp_filename) != NULL);
  VERIFY(saxs_document_curve_count(doc) == exp->exp_curve_count);
  VERIFY(saxs_document_property_count(doc) == exp->exp_property_count);

  cnt = 0;
  c = exp->exp_curves;
  sc = saxs_document_curve(doc);
  while (c && sc) {
    VERIFY(saxs_curve_type(sc) == c->exp_curve_type);
    VERIFY(saxs_curve_data_count(sc) == c->exp_curve_data_count);
    VERIFY(strstr(saxs_curve_title(sc), c->exp_curve_title) != NULL);

    c = c->next;
    sc = saxs_curve_next(sc);
    cnt++;
  }
  /* sanity check for .exp-file */
  VERIFY(cnt == exp->exp_curve_count);

  cnt = 0;
  p = exp->exp_properties;
  sp = saxs_document_property(doc);
  while (p && sp) {
    VERIFY(strcmp(saxs_property_name(sp), p->exp_property_name) == 0);
    VERIFY(strstr(saxs_property_value(sp), p->exp_property_value) != NULL);

    p = p->next;
    sp = saxs_property_next(sp);
    cnt++;
  }
  /* sanity check for .exp-file */
  VERIFY(cnt == exp->exp_property_count);

  return 0;
}


static int run_test(const char *infilename,
                    const char *outfilename,
                    const char *expfilename) {

  struct expect *exp = expect_get();
  VERIFY(expect_read(exp, expfilename) == 0);

  /* read and verify */
  saxs_document *doc = saxs_document_create();
  VERIFY(saxs_document_read(doc, infilename, NULL) == 0);
  verify(doc, exp);

  /* write, read and re-verify */
  if (outfilename) {
    VERIFY(saxs_document_write(doc, outfilename, NULL) == 0);

    saxs_document_free(doc);
    doc = saxs_document_create();
    VERIFY(saxs_document_read(doc, outfilename, NULL) == 0);
    verify(doc, exp);
  }

  expect_free(exp);
  saxs_document_free(doc);

  return EXIT_SUCCESS;
}

int main (int argc, char **argv) {
  switch (argc) {
    case 3:
      return run_test(argv[1], NULL, argv[2]);

    case 4:
      return run_test(argv[1], argv[2], argv[3]);

    default:
      fprintf(stderr, "Usage: %s <INFILE> [OUTFILE] <EXPFILE>\n", argv[0]);
      return EXIT_FAILURE;
  }
}
