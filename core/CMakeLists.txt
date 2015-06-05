# Set up the list of source files that are a part of the core.
set(CORE_SOURCE_FILES
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
