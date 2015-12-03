#include <stdio.h>
#include <string.h>

#include "saxsimage.h"

int main(int argc, char** argv){
  char *input = (argc > 1) ? argv[1] : "-";
  char *format = (argc > 2) ? argv[2] : NULL;
  printf("Reading '%s'\n", input);

  saxs_image *img = saxs_image_create();
  int result = saxs_image_read(img, input, format);
  printf("Result: %d (%s)\n", result, strerror(result));
  if (result == 0) {
    printf("Number of properties: %d\n", saxs_image_property_count(img));
    printf("Number of frames: %d\n", saxs_image_frame_count(img));
    printf("Height: %zu\n", saxs_image_height(img));
    printf("Width: %zu\n", saxs_image_width(img));
  }
  saxs_image_free(img);
  return 0;
}
