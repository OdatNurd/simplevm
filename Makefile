all: vm

#
# The list of C files to compile.
CFILES=main.c vm.c context.c
HEADERS=context.h

#
# The C compiler to use and the flags to pass it.
CFLAGS=-g -Wall
CC=gcc

#
# Create a set of object files thaat goes with the above.
OBJDIR=.obj
OBJS=$(addprefix $(OBJDIR)/, $(CFILES:.c=.o))

#
# Set up implicit rules
.PHONY: all clean
.SUFFIXES:
.SUFFIXES: .c

#
# Clean up cruft
clean:
	@echo Cleaning
	@rm -rf $(OBJDIR) vm

#
# Link the binary
vm: $(OBJS)
	@echo Linking
	@$(CC) $(OBJS) -o $@

#
# Create the directory the object files end up in
$(OBJDIR)/.x:
	@echo Making $(OBJDIR)
	@mkdir -p $(OBJDIR)
	@touch $(OBJDIR)/.x

#
# Compile C course files
$(OBJDIR)/%.o: %.c $(OBJDIR)/.x $(HEADERS)
	@echo Compiling $<
	@$(CC) -c $(CFLAGS) -o $@ $<

