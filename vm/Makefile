###############################################################################
#
# Specify the name of the project, which will be used to name the executable.
#
###############################################################################
NAME= vm


###############################################################################
#
# This specifies the type of project that this is.
#
###############################################################################
TARGET_TYPE= bin


###############################################################################
#
# Specify the source files for this binary. You only need to specify one of
# the three at a minimum, though you can use more than one if you need.
#
###############################################################################
MFILES=
CFILES= main.c
CPPFILES=


###############################################################################
#
# Specify any special compiler flags for this executable. The build system will
# usually provide all that you need, so these are only needed in special cases.
#
###############################################################################
TARGET_CFLAGS=
TARGET_MFLAGS=
TARGET_CPPFLAGS=


###############################################################################
#
# Specify the relative path to the root of this source tree (the path to the
# Makefiles directory). It'll be obvious if you get this wrong.
#
###############################################################################
BASEDIR= ..


###############################################################################
#
# Specify any special link flags here as needed for your project. In most cases
# this can be left empty.
#
###############################################################################
TARGET_LINK_FLAGS=
TARGET_LINK_POST=


###############################################################################
#
# Specify a list of subdirectories (assumed to be under the root of the current
# source tree) that contain library headers that need to be included. This is
# used if you store libraries not under the tree root directly or if you want
# to not have to specify the library name in the include directive. You might
# set this to "libsrc" if you store your libs in "treeroot/libsrc" instead of
# "treeroot", or you might set it to "mylib" if your library is being stored
# in "treeroot/mylib" but you don't want to include "mylib" in the include
# path.
#
###############################################################################
LIB_SUBDIRS=


###############################################################################
#
# If your binary links to libraries that require the Objective-C libraries
# to be linked, but none of the sources in the project are ObjC source files,
# then set this variable to YES to tell the build system that it should link
# with the ObjC support libraries even though it doesn't seem neccesary.
#
###############################################################################
OBJC_LINK=


###############################################################################
#
# Provide a list of static libraries that are a part of this source tree that
# this binary relies on. Specify just the project name of the project that
# creates the library. Your binary will relink if any of the libraries given
# here change after it has been linked.
#
###############################################################################
SLIBS= core


###############################################################################
#
# Provide a list of dynamic libraries that are a part of this source tree that
# this binary relies on. Specify just the project name of the project that
# creates the library.
#
###############################################################################
DLIBS=


###############################################################################
#
# Specify a list of libraries that your binary needs which aren't stored in
# this source tree. Specify here what you would provide in the -l line. These
# can be static or dynamic libraries, but note that your binary won't get
# automatically relinked if a static library in this list changes.
#
###############################################################################
OLIBS=


###############################################################################
#
# Provide a list of directories that should be created. This step happens
# before anything else in the makefile. The directories built are relative to
# the current directory unless you specify an absolute path.
#
###############################################################################
DIRECTORIES=


###############################################################################
#
# Provide a list of files to be copied somewhere, and the directory they should
# be copied to. The DIRECTORIES rule will be processed first, so it is safe to
# copy files with an OUTPUT_DIR that is set to a directory that will be
# created.
#
###############################################################################
COPYFILES=
OUTPUT_DIR=

###############################################################################
#
# Decide if we want builds to be verbose:
#   YES - Commands used to build the project are displayed
#   NO  - The build system just tells you what it is compiling/linking/etc
#
# Decide if build system problems should be colored or not:
#   YES - Compiler/linker warnings and errors are colored for emphasis
#   NO  - All output is normal
#
###############################################################################
VERBOSE_BUILDS= NO
COLOUR_WARNINGS= YES


###############################################################################
#
# Pull in the build system, which will build the project.
#
###############################################################################
include $(BASEDIR)/Makefiles/buildsystem.make

run: vm
	@vm