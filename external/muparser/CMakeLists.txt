
project (MUPARSER CXX)

set (MUPARSER_HEADERS muParserBase.h
                      muParserBytecode.h
                      muParserCallback.h
                      muParserDef.h
                      muParserDLL.h
                      muParserError.h
                      muParserFixes.h
                      muParser.h
                      muParserInt.h
                      muParserStack.h
                      muParserTemplateMagic.h
                      muParserToken.h
                      muParserTokenReader.h)

set (MUPARSER_SOURCES muParserBase.cpp
                      muParserBytecode.cpp
                      muParserCallback.cpp
                      muParser.cpp
                      muParserDLL.cpp
                      muParserError.cpp
                      muParserInt.cpp
                      muParserTokenReader.cpp)

add_definitions (-DMUPARSERLIB_EXPORTS -DMUPARSER_DLL)

add_shared_library (muparser
                    SOURCES ${MUPARSER_SOURCES} ${MUPARSER_HEADERS}
                    VERSION 2.2.2)

install_library (TARGETS muparser)
