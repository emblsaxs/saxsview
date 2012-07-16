//
// TODO: This converts documents only so far, add image conversion.
//

#include "saxsdocument.h"
#include "saxsdocument_format.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>

static void
usage() {
  char infmt[1024] = { '\0' }, outfmt[1024] = { '\0' };

  saxs_document_format *fmt = saxs_document_format_first();
  while (fmt) {
    if (fmt->read)
      sprintf(infmt, "%s\n  %-20s %s (.%s)", infmt, fmt->name, fmt->description, fmt->extension);

    if (fmt->write)
      sprintf(outfmt, "%s\n  %-20s %s (.%s)", outfmt, fmt->name, fmt->description, fmt->extension);

    fmt = saxs_document_format_next(fmt);
  }

  fprintf(stdout,
          "Usage: svconv [OPTIONS] <INFILE> <OUTFILE>\n"
          "Convert SAXS documents or images from one format to another.\n"
          "\n"
          "Supported input formats: %s\n"
          "\n"
          "Supported output formats: %s\n"
          "\n"
          "Mandatory arguments to long options are mandatory for short options too.\n"
          "\n"
          "Known Options:\n"
          "      --informat=<FORMAT>  Read INFILE assuming FORMAT.\n"
          "      --outformat=<FORMAT> WRITE OUTFILE in FORMAT.\n"
          "\n"
          "  -v, --version            print version information and exit\n"
          "  -h, --help               print this help text and exit\n"
          "\n"
          "Report bugs to <%s>.\n", infmt, outfmt, PROJECT_BUGREPORT);

  exit(EXIT_SUCCESS);
}

static void
version() {
  fprintf(stdout, "svconf 0.1 (r%s)\n"
                  "Copyright (c) Daniel Franke 2012\n", "0");

  exit(EXIT_SUCCESS);
}


static void
getopt(int argc, char **argv, char** informat, char **infile,
                              char **outformat, char **outfile) {
  int opt;
  int option_index = 0;

  static struct option long_options[] = {
    { "informat",  required_argument, 0, 1 },
    { "outformat", required_argument, 0, 2 },
    { "version",   no_argument,       0, 'v' },
    { "help",      no_argument,       0, 'h' },
    { 0, 0, 0, 0}
  };

  while((opt = getopt_long(argc, argv, "vh", long_options,
                           &option_index)) != -1) {
    switch(opt) {
      case 1:
        *informat = optarg;
        break;

      case 2:
        *outformat = optarg;
        break;

      case 'v':
        version();

      case 'h':
        usage();

      default:
        // getopt prints a message already, just exit
        exit(EXIT_FAILURE);
    }
  }

  if (argc - optind != 2) {
    fprintf(stderr, "svconv: exactly to arguments expected, "
                    "see `svconv --help` for details.\n");
    exit(EXIT_FAILURE);
  }

  *infile  = argv[optind++];
  *outfile = argv[optind++];
}



int main(int argc, char **argv) {
  char *informat = 0L, *infile = 0L, *outformat = 0L, *outfile = 0L;
  getopt(argc, argv, &informat, &infile, &outformat, &outfile);

  if (informat) {
    saxs_document_format *fmt = saxs_document_format_find_first(infile, informat);
    if (!fmt || !fmt->read) {
      fprintf(stderr, "svconv: unknown or unhandled input format '%s', "
              "see `svconv --help` for details.\n", informat);
      exit(EXIT_FAILURE);
    }
  }

  if (outformat) {
    saxs_document_format *fmt = saxs_document_format_find_first(outfile, outformat);
    if (!fmt || !fmt->write) {
      fprintf(stderr, "svconv: unknown or unhandled output format '%s', "
              "see `svconv --help` for details.\n", outformat);
      exit(EXIT_FAILURE);
    }
  }
  
  saxs_document *doc = saxs_document_create();
  int res = saxs_document_read(doc, infile, informat); 
  if (res != 0) {
    fprintf(stderr, "%s: parse error (%s)\n", infile, strerror(res));
    return EXIT_FAILURE;
  }

  res = saxs_document_write(doc, outfile, outformat); 
  saxs_document_free(doc);

  if (res != 0) {
    fprintf(stderr, "%s: write error (%s)\n", infile, strerror(res));
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
