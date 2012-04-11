project (CBFLIB C)

set (CBFLIB_HEADERS cbf.h cbf_alloc.h cbf_ascii.h cbf_binary.h
                    cbf_byte_offset.h cbf_canonical.h cbf_codes.h
                    cbf_compress.h cbf_context.h cbf_file.h cbf_lex.h
                    cbf_packed.h cbf_predictor.h cbf_read_binary.h
                    cbf_read_mime.h cbf_simple.h cbf_string.h cbf_stx.h
                    cbf_tree.h cbf_uncompressed.h cbf_write.h
                    cbf_write_binary.h global.h md5.h)

set (CBFLIB_SOURCES cbf.c cbf_alloc.c cbf_ascii.c cbf_binary.c
                    cbf_byte_offset.c cbf_canonical.c cbf_codes.c
                    cbf_compress.c cbf_context.c cbf_file.c cbf_lex.c
                    cbf_packed.c cbf_predictor.c cbf_read_binary.c
                    cbf_read_mime.c cbf_simple.c cbf_string.c
                    cbf_stx.c cbf_tree.c cbf_uncompressed.c cbf_write.c
                    cbf_write_binary.c md5c.c)

saxsview_add_library(cbf SOURCES ${CBFLIB_HEADERS} ${CBFLIB_SOURCES}
                         LIBRARIES m
                         VERSION 0.7.9.1
                         SHARED)

saxsview_install (TARGETS cbf)

