

#ifndef LIBSAXSDOCUMENT_SAXSDOCUMENT_FORMAT_H
#define LIBSAXSDOCUMENT_SAXSDOCUMENT_FORMAT_H

struct saxs_document;

struct saxs_document_format {
  int (*read)(struct saxs_document *doc, const char *filename);
  int (*write)(struct saxs_document *doc, const char *filename);
};
typedef struct saxs_document_format saxs_document_format;


saxs_document_format*
saxs_document_format_find(const char *filename, const char *format);


/** Case-insensitive string comparison. */
int compare_format(const char *a, const char *b);

/** Extract the suffix of a filename, e.g. 'bsa.dat' has suffix 'dat'. */
const char* suffix(const char *filename);

#endif /* !LIBSAXSDOCUMENT_SAXSDOCUMENT_FORMAT_H */
