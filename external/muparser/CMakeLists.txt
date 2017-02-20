
project (MUPARSER CXX)

set (MUPARSER_SOURCES muParserBase.cpp
                      muParserBytecode.cpp
                      muParserCallback.cpp
                      muParser.cpp
                      muParserDLL.cpp
                      muParserError.cpp
                      muParserInt.cpp
                      muParserTokenReader.cpp)


add_shared_library (muparser
                    SOURCES ${MUPARSER_SOURCES}
                    VERSION 2.2.2)

target_compile_definitions (muparser PRIVATE MUPARSERLIB_EXPORTS MUPARSER_DLL)
target_include_directories (muparser PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

install_library (TARGETS muparser)
