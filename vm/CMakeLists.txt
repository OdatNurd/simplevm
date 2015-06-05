set(VM_SOURCE_FILES
    context.c
    context.h
    main.c
    opcodes.h
    registers.h
    vm.c
    vm.h)

add_executable(vm ${VM_SOURCE_FILES})
