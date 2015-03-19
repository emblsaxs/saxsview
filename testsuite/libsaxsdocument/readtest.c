
#include <stdio.h>
#include <string.h>

#include "saxsdocument.h"

int main(int argc, char** argv){
  char *input = (argc > 1) ? argv[1] : "-";
  char *format = (argc > 2) ? argv[2] : NULL;
  printf("Reading '%s'\n", input);

  saxs_document *doc = saxs_document_create();
  int result = saxs_document_read(doc, input, format);
  printf("Result: %d (%s)\n", result, strerror(result));
  if (result == 0) {
    printf("Number of properties: %d\n", saxs_document_property_count(doc));
    printf("Number of curves: %d\n", saxs_document_curve_count(doc));
  }
  saxs_document_free(doc);
  return 0;
}