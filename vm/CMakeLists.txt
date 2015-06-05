# Set up the source files for this project.
set(VM_SOURCE_FILES
    main.c
    )

# Indicate that these source files should be used to generate a binary named vm
add_executable(vm ${VM_SOURCE_FILES})

# Indicate that we should link to the core library. This also indicates that the
# include directories for the core library should be added for this target.
target_link_libraries(vm LINK_PUBLIC core)
