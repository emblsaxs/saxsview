/*
 * Test functions from columns.h
 * 
 * TODO: use some kind of testing framework rather than
 *       hand-rolling everything
 */

#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "columns.h"

static void test_lines_printf(){
  struct line *l;

  l = lines_create();
  assert_valid_line(l);

  lines_printf(l, "a = %.1f, b=%.2e", 1.234, 56.789);
  assert_valid_line(l);
  assert(0 == strcmp(l->line_buffer, "a = 1.2, b=5.68e+01"));

  lines_printf(l, "%s %s %s",
               "A very long line with lots of characters",
               "to test the ability of lines_printf to cope",
               "with lines longer than the default buffer size");
  assert_valid_line(l);

  lines_printf(l, "oldbuffer");

  /* Check the ability to use the old buffer as input */
  lines_printf(l, "Previous buffer was '%s' i.e. old '%s'",
               l->line_buffer, (l->line_buffer)+3);
  assert_valid_line(l);

  assert(0 == strcmp(l->line_buffer,
         "Previous buffer was 'oldbuffer' i.e. old 'buffer'"));

  lines_free(l);
}


int main(int argc, char ** argv){
  printf("Testing lines_printf...\n");
  test_lines_printf();

  printf("All tests completed successfully!\n");
  return 0;
}