
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <columns.h>
#include <saxsdocument.h>
#include <saxsdocument_format.h>

int main(int argc, char** argv){
  char *input = (argc > 1) ? argv[1] : "-";
  char *format = (argc > 2) ? argv[2] : NULL;
  assert(input);

  struct saxs_document_format const *fmt1;
  if (format) {
    fmt1 = saxs_document_format_find_first(input, format);
    if (!fmt1) {
      printf("No format available for filename '%s', format '%s'\n", input, format);
      return 0;
    }
  } else {
    fmt1 = saxs_document_format_first();
  }
  assert(fmt1);

#ifdef __AFL_HAVE_MANUAL_CONTROL
  while (__AFL_LOOP(1000)) {
#else
  if (format) {
    printf("Reading '%s' with format '%s'...\n", input, format);
  } else {
    printf("Reading '%s'...\n", input);
  }
  fflush(stdout);
#endif

  int rc = 0;
  struct line *lines = NULL;

  rc = lines_read(&lines, input);
  if (rc) {
#ifdef __AFL_HAVE_MANUAL_CONTROL
    continue;
#else
    printf("lines_read returned %d (%s)\n", rc, strerror(rc));
    return 0;
#endif
  }
  assert(lines);

  struct saxs_document_format const *fmt = fmt1;
  while (fmt) {
    saxs_document *doc = saxs_document_create();
    assert(doc);
    if (fmt->read) {
#ifndef __AFL_HAVE_MANUAL_CONTROL
      printf("Trying format '%s'...\n", fmt->name);
      fflush(stdout);
#endif
      rc = fmt->read(doc, lines, NULL);
#ifndef __AFL_HAVE_MANUAL_CONTROL
      printf("Result: %d (%s)\n", rc, strerror(rc));
      if (rc == 0) {
        printf("Number of properties: %d\n", saxs_document_property_count(doc));
        printf("Number of curves: %d\n", saxs_document_curve_count(doc));
      }
#endif
    }
    saxs_document_free(doc);
    if (format) {
      fmt = saxs_document_format_find_next(fmt, input, format);
    } else {
      fmt = saxs_document_format_next(fmt);
    }
  }

  lines_free(lines);
#ifdef __AFL_HAVE_MANUAL_CONTROL
  }
#endif

  return 0;
}