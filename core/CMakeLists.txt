# Set up the list of source files that are a part of the core.
set(CORE_SOURCE_FILES
    config.h
    context.c
    context.h
    opcodes.h
    registers.h
    vm.c
    vm.h)

# Indicate that these files should be linked into a static library named core (well, libcore.a).
add_library(core STATIC ${CORE_SOURCE_FILES})

# Set the directory below this one as a public include directory. Anything that links 
# against this library will have this path added to its include path. We use .. at the end
# so that includes require the <core/> prefix.
target_include_directories(core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")

# Set the version number of the core library.
#
# This is in here as an example of how to get CMake to generate files. 
set (core_VERSION_MAJOR 0)
set (core_VERSION_MINOR 1)
set (core_VERSION_PATCH 0)

# Configure the file. This pulls the input file and generates it to the output file.
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/config.h"
    )
