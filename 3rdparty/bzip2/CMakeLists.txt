add_definitions(-D_FILE_OFFSET_BITS=64 -fPIC)

if (WIN32)
    add_definitions(-DWIN32 -D_WIN32)
endif()

set(bzip2_SRCS
    blocksort.c
    huffman.c
    crctable.c
    randtable.c
    compress.c
    decompress.c
    bzlib.c)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

# final lib
add_library(bzip2_hemera_static STATIC ${bzip2_SRCS})
set_target_properties(bzip2_hemera_static PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
                      LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
