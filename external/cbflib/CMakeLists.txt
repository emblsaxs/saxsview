project (CBFLIB C)

set (CBFLIB_SOURCES cbf.c cbf_alloc.c cbf_ascii.c cbf_binary.c
                    cbf_byte_offset.c cbf_canonical.c cbf_codes.c
                    cbf_compress.c cbf_context.c cbf_file.c cbf_lex.c
                    cbf_packed.c cbf_predictor.c cbf_read_binary.c
                    cbf_read_mime.c cbf_simple.c cbf_string.c
                    cbf_stx.c cbf_tree.c cbf_uncompressed.c cbf_write.c
                    cbf_write_binary.c md5c.c)

add_shared_library (cbf
                    SOURCES ${CBFLIB_SOURCES}
                    LIBRARIES m
                    VERSION 0.7.9.1)
target_include_directories(cbf PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

install_library (TARGETS cbf)

