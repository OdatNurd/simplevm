###############################################################################
#
# Buildsystem
#  - Nov 30/2004  Terence Martin (poindexter@nurdz.com)
#     - Initially created
#
#  - Dec 1/2004   Terence Martin (poindexter@nurdz.com)
#     - Add the concept of LIB_SUBDIRS so that it is possible to store a 
#       library project in a directory other than the root of the source tree.
#       Any directories added to LIB_SUBDIRS are added to the include path.
#
#  - Dec 2/2004   Terence Martin (poindexter@nurdz.com)
#     - Add in the concept of the VERSION variable, which pulls in the contents
#       of the file named ReleaseNumber in the same directory as the
#       buildsystem.make file. This will be defined as VERSION for all targets
#       as a string, and also as VERSION_OBJC for ObjC files, in which case it
#       is an NSConstantString (though ObjC also gets the regular string
#       version).
# 
#  - Dec 6/2004   Terence Martin (poindexter@nurdz.com)
#     - Remove VERSION_OBJC, and add in OBJC_LINK as an option to force a
#       binary to link to GNUstep and libobjc even if none of the source files
#       in the project are ObjC source files.
#
#  - Dec 9/2004   Terence Martin (poindexter@nurdz.com)
#     - Added a special rule to the binary rules so that when a binary is
#       linked, a symlink to it is placed in the project directory, but only
#       if there is no file with that name, or there is a file and it's a
#       symlink already. A rule was added to clean to clean the file, but again
#       only if it is a symlink.
#
#  - Jun 3/2011   Terence Martin (poindexter@nurdz.com)
#     - Add in the concept of the BUILD_HOST variable, which indicates the
#       hostname the compile is happening on.
# 
###############################################################################
#
# The OBJC portions of this buildsystem assume:
#
#   - That you are using gcc 3+, OR you are using gcc 2 and you have installed
#     a new libobjc that actually works (the libobjc that comes with pre 3.0
#      versions of GCC doesn't work in a threaded environment).
#   
#   - That you are using gnustep-base 1.10.0. That is, we assume if you're
#     using objc you are using gnustep, since objc is at it's coolest when you
#     have a foundation library. In particular, if you are not using this exact
#     version of gnustep, the library and header locations may be in a 
#     different place.
#
###############################################################################
#
# Other Assumptions:
#
#  1) Unless you set the CC or CXX variables prior to including this makefile,
#     they will use the default values that gnu make provides. 
#     (CC is for C and ObjC, CXX is for C++)
#
#  2) A binary can only be dependant on static libraries in the project tree,
#     since there is no need to rebuild a binary if a shared object file is
#     changed, and since it is considered unlikely that you change system 
#     libraries often enough for us to make dependencies on them. Also, it's a
#     pain to figure out where they are,
#
#  3) When a binary is linked with a shared object file that is stored in the
#     source tree (DLIBS below), the binary is flagged with the location of the
#     library directory of this source tree, relative to where the binary is
#     stored. This means that when the binary is run, it will find the shared
#     library for the source tree that it is stored in. This is handy if there
#     is more than one version of the source tree around, as it means the 
#     binary will always associate with the libraries compiled from the same
#     source tree as itself.
#
#     The downside to this is that due to the relative nature of the rpath,
#     if you run the binary and give a relative path to it, the library won't
#     be found. The solution to this is to always run the binary from the bin
#     directory (use a wrapper script to do the actual execution, or put the
#     bin directory in your path).
#
#     Normally, if you had multiple versions of the same source tree present,
#     one of them would be flagged as current using some sort of symlink, and
#     the bind directory that follows the symlink would be in the path. Thus
#     the binaries for the working tree are always in the path.
#
###############################################################################
#
# This makefile represents a build system to make construction of makefiles
# for projects easier to do. You only need to configure a few simple variables 
# and include this file to get the output that you desire.
#
# In general, the directory tree used by this buildsystem looks something like 
# this:
#
# .../SourceTreeName/
#    |
#    \_ Makefiles/ <-- Contains the build system
#    |
#    \_ bin/       <-- Contains all binaries for binary projects in this tree
#    |
#    \_ lib/       <-- Contains all libraries for library projects in this tree
#    |
#    \_ Project1/  <-- Some project
#    |
#    \_ Project2/  <-- Some project
#    |
#    \_ Makefile   <-- This toplevel makefile builds all projects
#
#
# The bin and lib directories will be automatically created when they are
# needed if they don't already exist. The Makefiles directory contains this 
# file and any files it might need to do it's job.
#
###############################################################################
#
# The build system provides two rules for you: install and clean. The default
# is install, which is what actually builds binaries and libraries. Note that
# here install means "install into the bin and lib directory of the current
# source tree". If you want the binaries and/or libraries copied elsewhere
# (i.e. a system directory) you have to do that yourself.
#
# The clean rule does the following:
#   - Remove all object files and dependency files for any sources in the
#     current project.
#
#   - For binaries and libraries, the compiled binary/library is removed from
#     the bin/lib directory. For binaries, if there is a symlink to the binary
#     in a project directory, it will be removed.
#
#   - For DIRECTORIES targets (see below), the directories provided are removed,
#     but only if they are empty.
#
#   - For COPYFILES targets (see below), the copied files are removed, but only
#     if they are identical to the version in the project directory (so that
#     modified files get left behind).
#
#   - The bin and lib directories are removed, if they are empty.
#
#
# Because of the cleaning rules that will only remove a directory if it is
# empty, and the order in which the rules may be applied, a single "make clean"
# operation will get rid of files, but might leave some directories laying
# around. For a true clean operation, you need to issue make clean twice.
#
###############################################################################
#
# Based on the above source tree picture, the following should always be true:
#   o All projects stored under a source tree name are all somehow related.
#     i.e. the binaries provide some service as a whole, they need the required 
#     libraries to function, etc. 
#
#   o If a project produces a binary, it will be copied to the bin directory 
#     when it is created/updated (though a copy will be left in the project 
#     directory as well)
#  
#   o If a project produces a library, it will be copied to the lib directory 
#     when it is created/updated (though a copy will be left in the project 
#     directory as well)
#
#   o For a library project, the source files and include files will be 
#     contained in the same directory, such that any project that includes a 
#     library Bob, it can do:
#               #include <Bob/Header.h>
#     to include headers for the Bob library.
#
#   o Project subdirectories are not constrained to being just an immediate 
#     subdirectory of the source tree, they can be arranged in any logical 
#     order.
#
#   o Because of rule #1 above (all projects are interrelated) the build system 
#     assumes that the release structure looks like the build tree (the bin and
#     lib directories will be stored at the same directory level). This has
#     specific impact if dynamic libraries are generated, since the binaries
#     that are dynamically linked with them will assume that they are stored in
#     bin/../lib/*.
#
#     This property ensures that a source tree is completely self contained, 
#     even in cases where there are several working copies existing at the same 
#     time (so binaries will only use libraries associated with the same source 
#     tree as themselves)
#
###############################################################################
#
# A Makefile in a project directory may contain the following variables (only
# NAME, TARGET_TYPE, BASEDIR and one of MFILES, CFILES or CPPFILES is
# strictly required):
#
# NAME              * Name of the binary or library produced. For libraries, do
#                     not include the "lib" prefix, or a filename suffix. That is 
#                     added for you.
#
# TARGET_TYPE       * One of 'bin', 'lib' or 'so' to specify if this produces a 
#                     binary or a library (for 'so', a shared object). If you 
#                     want to build a makefile that only copies files or builds 
#                     some directories, you can set this to 'copy'. 
#
# MFILES            * A whitespace separated list of .m (ObjC) files used to make 
#                     NAME. This is optional if there are CFILES or CPPFILES.
#
# CFILES            * A whitespace separated list of .c (C) files used to make 
#                     NAME. This is optional if there are MFILES or CPPFILES.
#
# CPPFILES          * A whitespace seperated list of .cpp (C++) files used to 
#                     make NAME. This is optional if there are MFILES or CFILES.
#
# OBJC_LINK         * When this is set to YES, the link step of a binary target
#                     will link with the Objective-C runtime library and GNUstep.
#                     This is automatically set to YES by the build system if
#                     MFILES is not empty. You may need to force this to be yes
#                     if you are linking a binary to a library that contains 
#                     ObjC code and the binary doesn't contain any ObjC sources
#                     in it.
#
#                     This is ignored for non-binary targets.
#
# TARGET_LINK_FLAGS * For most cases, the build system specifies all link options
#                     that your project is likely to need. However, you may find
#                     yourself needing to specify options to the linker yourself.
#                     Those can be placed here (note that we link by calling gcc).
#                     This might be useful if you are using an outside library.
#
# TARGET_LINK_POST  * As above, but comes at the end of the linker command line
#                     prior to the command that outputs the final binary/library.
#
# TARGET_CFLAGS     * If you need to add any compilation flags, put them in these
# TARGET_MFLAGS       variables. There is one for each type of compilation.
# TARGET_CPPFLAGS    
#
# BASEDIR           * The base of the build system. This should be relative path 
#                     information that gets you from whatever project you are in 
#                     to the root of the current tree.
#
# SLIBS             * A list of static libraries in the current source tree that
#                     NAME needs to link to. Do not specify the "lib" prefix or
#                     a suffix. All libraries listed here are assumed to be 
#                     stored in the $(BASEDIR)/lib directory, and be static, so 
#                     that when you change a static library, the build system can 
#                     tell and relink.
#
# DLIBS             * A list of dynamic libraries in the current source tree that
#                     NAME needs to link to. Do not specify the "lib" prefix or
#                     a suffix. All libraries listed here are assumed to be 
#                     stored in the $(BASEDIR)/lib directory.
#
# OLIBS             * A list of libraries (static or otherwise) that NAME needs to
#                     link to. Do not specify the "lib" prefix or a suffix. These
#                     libraries are outside libraries (i.e. not libraries that are
#                     part of the current source tree)
#
# LIB_SUBDIRS       * A list of directories inside the current source tree which
#                     contain library include files that this project needs. The
#                     root of the source tree is always considered, so that if
#                     your library is in treeroot/mylib, you can include with
#                     #include <mylib/myheader.h> with no problems. If you would
#                     rather #include <myheader.h>, then you would need to add
#                     the directory mylib to LIB_SUBDIRS.
#
#                     Additionally, this can be used if you lay out your lib
#                     sources so that they are not direct children of the tree
#                     root (e.g. treeroot/libsources/staticlibs/mylib). In this
#                     case, you would need to add "libsources/staticlibs" to
#                     this variable.
#
# DIRECTORIES       * This can be set to a list of directory names which the build
#                     system will create for you. The list can have 1 or more
#                     directories, which will be created as if you were using the
#                     mkdir command yourself (that is, they'll end up in the
#                     directory make is running in if they're not somehow relative
#                     or absolute.
#
# COPYFILES         * This is a list of files to be copied to the location given
# OUTPUT_DIR          in OUTPUT_DIR. The copy will only take effect if the file
#                     in the output dir doesn't exist or hasn't been modified. If
#                     a file with this name exists in OUTPUT_DIR and has a later
#                     timestamp than the file to be copied, the copy does not 
#                     occur.
#                   
#                     Note that directories specified in the DIRECTORIES target
#                     will always be created before the COPYFILES rule takes 
#                     effect, so it is safe to copy files to a directory made by 
#                     DIRECTORIES.
# 
# VERBOSE_BUILDS    * Set to YES or NO to determine if you want verbose builds.
#                     A verbose build outputs the commands it is executing while
#                     they are occuring, whereas a non-verbose build just tells 
#                     you what files it is compiling/linking/etc. If not given,
#                     the default is NO
#
# COLOUR_WARNINGS   * Set to YES or NO to determine if you want errors and 
#                     warnings to be output in color. When YES, any output that 
#                     the compiler generates will be colored to draw attention to 
#                     it. The default is YES.
#                     
#
# After the above variables are set, end the makefile with:
#
#      include $(BASEDIR)/Makefiles/buildsystem.make   
#
# This will include the buildsystem makefile (this file), which does the rest 
# of the work neccesary to build your project and put it into the appropriate 
# locations.
#
###############################################################################
#
# When compiling files, the buildsystem defines one of the following variables:
#   __OBJC__, __C__, __CPP__
#
# These can be used to determine how the file is being compiled. These would
# normally be used in header files, so that a single header file can be used
# for a library containing different types of code.
#
# Additionally, the following are defined:
#   VERSION      (a string)
#   REVISION     (a string)
#   BUILD_HOST   (a string)
#
# VERSION is defined by taking the first line of the ReleaseNumber file, 
# removing leading and trailing whitespace, and then compressing all other
# whitespace into a single underscore. The version number is controlled 
# directly by you, and is only altered when you manually increment it.
#
# REVISION is defined by asking subversion the revision number of the
# working copy that is currently in use. If this information can't be
# obtained, revision ends up being 0. The revision number is directly
# controlled by SubVersion. Although the revision number is enough to
# uniquely specify the code base in use, a user defined version number is
# usually also provided.
#
# BUILD_HOST is defined by asking the system what it's hostname is (via the
# hostname command itself). 
#
# The revision number may be a number or two numbers seperated by colons,
# and may have trailing characters such as M or S. The revision is obtained
# using svnversion, so see the help for that program for more information.
# In general, when the revision number isn't just a straight number, the
# working copy is a possibly out of date or mixed environment, and should
# not be used for releases.
#
###############################################################################
#
# End of buildsystem.make documentation. Stop reading here. No user serviceable
# parts inside.
#
###############################################################################

#
# Fix the passed in strings for our own use.
#
INTERNALNAME=$(strip $(NAME))
PROJECT_TYPE=$(strip $(TARGET_TYPE))


#
# Create the version  and revision string.
#
VERSION= \"$(shell $(BASEDIR)/Makefiles/getVersion $(BASEDIR))\"
REVISION= \"$(shell $(BASEDIR)/Makefiles/getRevision $(BASEDIR))\"
BUILD_HOST= \"$(shell hostname)\"


#
# Create the list of files to be copied by prefixing them with the directory
# in which they should be stored.
#
COPYFILE_OUTPUT= $(addprefix $(OUTPUT_DIR)/, $(COPYFILES))

#
# Should builds be verbose?
#
VERBOSE_BUILDS?=NO


#
# Should warnings and errors be in color?
#
COLOUR_WARNINGS?=YES


#
# Use the VERBOSE_BUILDS setting to determine how we're going
# to build things.
#
ifeq "$(VERBOSE_BUILDS)" "YES"
SHOWCOMP=
ECHOSTAT=@true
else 
SHOWCOMP=@
ECHOSTAT=@echo
endif


#
# Use the COLOUR_WARNINGS settings to determine how to color things (if at all);
# 
# ESC[#;m where # is the colour code.  The #; sequence can be repeated.
#  0 Off           1 Bold          4 Underscore   5 Blink         7 Reverse
#  8 Concealed    30 Black   FG   31 Red    FG   32 Green FG     33 Yellow  FG
# 34 Blue FG      35 Magenta FG   36 Cyan   FG   37 White FG     40 Black   BG
# 41 Red  BG      42 Green   BG   43 Yellow BG   44 Blue  BG     45 Magenta BG
# 46 Cyan BG      47 White   BG
#
ifeq "$(COLOUR_WARNINGS)" "YES"
COLOUR_START=@echo -n "[31;1m"
COLOUR_END=@echo -n "[0;m"
else
COLOUR_START=
COLOUR_END=
endif


#
# Do we want Static or Dynamic Libraries? Dynamic libraries if the project
# type is so (shared object). If we do that, change the project type to be
# lib, as the rules are the same either way, and knowing wether this is static
# or not is all we need.
#
ifeq "$(PROJECT_TYPE)" "so"
STATIC=NO
PROJECT_TYPE= lib
else
STATIC=YES
endif


# 
# Specify some directories where we stash things:
#  LIBDIR is where libraries go when they are made
#  BINDIR is where binaries go when they are linked
#  OBJDIR is where build specific files are stored (these are per project).
#
LIBDIR=$(BASEDIR)/lib
BINDIR=$(BASEDIR)/bin
OBJDIR=.obj


#
# Some system commands that we want to use. Note that CC and CXX are not
# set here, they are taken from the environment or make's defaults.
#
AR=ar
RANLIB=ranlib


#
# Get the list of object files we want to work with. User can specify files
# for the three languges we support. Note that because of the way we create
# the list, if someone specifies the wrong kind of file in the file list
# (like say a .cpp file in the MFILES), then the list of object files is empty
# and nothing gets built, which is a warning to you that you've done something
# wrong.
#
OBJS=$(MFILES:.m=.o) $(CFILES:.c=.o) $(CPPFILES:.cpp=.o)


#
# Determine if there were any ObjC files
#
ifeq "$(strip $(MFILES))" ""
OBJC_CODE=NO
else
OBJC_CODE=YES
OBJC_LINK=YES
endif


#
# Determine if there were any C++ files
#
ifeq "$(strip $(CPPFILES))" ""
CPP_CODE=NO
else
CPP_CODE=YES
endif


#
# Use this to suppres: "type and size of dynamic symbol BLAH are not defined"
#
# These get generated in certain situations (old GCC versions, 3.* doesn't seem
# to have a problem).
#
# What we do is grep to see if the linker outputs anything. We output 
# everything the linker says EXCEPT these errors. The trick is we need to 
# propogate the linker's return value so make knows what's up. To do that we
# use the knowledge that grep -v succeds if it grepped anything out, and use
# test to invert the return value. This works because the linker only outputs
# text on an error.
#
SUPPRESS_GNUSTEP_WARNING=2>&1 |grep -v "warning: type and size of dynamic symbol.*are not defined" | grep ".*" ; test "$$? -ne 0"


#
# For ObjC code, tell the VPATH mechanism where the gnustep headers are. 
#
ifeq "$(OBJC_CODE)" "YES"
VPATH+= $(GNUSTEP_SYSTEM_ROOT)/Library/Headers/
SYS_INCLUDES+= -isystem $(GNUSTEP_SYSTEM_ROOT)/Library/Headers/
endif


#
# If OBJC_LINK is set to YES, then set options to ensure that any link step
# that is invoked will link in the libraries needed by Objective-C programs
# (the ObjC runtime and GNUstep base).
#
# Additionlly, we need extra link flags when there is ObjC code, because
# ObjC is dynamic. What this means for us is that the linker may decide not to
# include code because it isn't directly called, but really it could be 
# called at runtime. So for ObjC, make sure the linker knows to just include
# everything we give it.
#
ifeq "$(OBJC_LINK)" "YES"
INCLUDED_LIBS+= -lobjc -lgnustep-base 
LINK_FLAGS+= -L$(GNUSTEP_SYSTEM_ROOT)/Library/Libraries/ 
OBJC_LINK_START= -Wl,--whole-archive
OBJC_LINK_STOP= -Wl,--no-whole-archive
endif


#
# For C++ code, include the libraries needed to link C++.
#
ifeq "$(CPP_CODE)" "YES"
INCLUDED_LIBS+= -lstdc++
endif


#
# Make a list of all libraries needed for this project as specified in the
# makefile.
#
PROJECT_LIBS= $(SLIBS) $(DLIBS) $(OLIBS)


#
# Make a list of all libraries that need to be linked with. We need to
# add the -l prefix for them. This makes INCLUDED_LIBS the complete list
# of libraries that we want to link with.
#
INCLUDED_LIBS+= $(addprefix -l, $(PROJECT_LIBS))


#
# Add the special object directory prefix to the object files that we're going
# to be needing for this project.
#
# We also need to create a list of dependency files, which are also in the object
# directory, and named based on the source files we're compiling.
#
LOCAL_OBJS=$(addprefix $(OBJDIR)/, $(OBJS))
DFILES=$(addprefix $(OBJDIR)/, $(MFILES:.m=.d)) $(addprefix $(OBJDIR)/, $(CFILES:.c=.d)) $(addprefix $(OBJDIR)/, $(CPPFILES:.cpp=.d))


#
# The base directory and directories in LIB_SUBDIRS contains all library 
# projects in the system, so make sure that make knows to look there for VPATH 
# info.
#
VPATH+= $(BASEDIR)/ $(addprefix $(BASEDIR)/,$(LIB_SUBDIRS))

#
# Set up system includes by assuming that the VPATH knows where all of our
# dependencies might be located.
#
INCLUDES= $(addprefix -I, $(VPATH))


#
# Set up the flags for warnings that we want in our code.
#
WARNING_FLAGS= -Wall -Wno-unknown-pragmas -W -Wno-unused-parameter -Wpointer-arith -Wcast-align -Winline -Wno-sign-compare -Wno-conversion


# 
# Set up compile flags for each of the languges we support.
#
ALL_MFLAGS+= -g -fconstant-string-class=NSConstantString -Wno-import $(WARNING_FLAGS) $(TARGET_MFLAGS) $(SYS_INCLUDES) $(INCLUDES) 
ALL_CFLAGS+= -g $(WARNING_FLAGS) $(TARGET_CFLAGS) $(SYS_INCLUDES) $(INCLUDES)
ALL_CPPFLAGS+= -g $(WARNING_FLAGS) $(TARGET_CPPFLAGS) $(SYS_INCLUDES) $(INCLUDES)


#
# Add to the link flags the library directory for this source tree, so that
# the linker can find any libraries it needs that are there.
#
LINK_FLAGS+=-L$(LIBDIR)


#
# The rules for static and dynamic libaries are different. In particular,
# the output filename is different, and you need extra flags to compile 
# a shared object. 
#
# This sets up what the resulting library would be named, if this is a
# lib target.
#
ifeq "$(STATIC)" "YES"
LIB=$(LIBDIR)/lib$(INTERNALNAME).a
else 
LIB=$(LIBDIR)/lib$(INTERNALNAME).so
ALL_MFLAGS+= -fPIC
ALL_CFLAGS+= -fPIC
ALL_CPPFLAGS+= -fPIC
endif


#
# The default rule is install, so that everything will get built and then
# propogated over to the proper place.
#	
default:  install 

#
# Start with with a rule that makes any directories and copy files want to
# happen.
#
install:: $(DIRECTORIES) $(COPYFILE_OUTPUT)


#
# Start with a clean rule. For each directory specified, try to remove it
# if it's empty. For each file copied, if it's not changed from what was
# installed, then erase it.
#
clean::
	$(foreach DIR, $(DIRECTORIES),$(shell if [ -d $(DIR) ] ; then rmdir -p --ignore-fail-on-non-empty $(DIR); fi))
	$(foreach FILE, $(COPYFILE_OUTPUT),$(shell cmp -s $(FILE) $(notdir $(FILE)); if [ $$? -eq 0 ] ; then rm -f $(FILE); fi))

#
# The release rule is used only for binaries, and strips them so that we can
# package them. All project types get this so that you can "make release" from a
# clean tree and get something that works.
#
release:: install


# 
# If the goal of make isn't the clean target, then include the dependency
# files now. The -include makes make not complain if the file doesn't exist
# yet (which it won't, if this is the first time make was called). We don't
# want to include the dependency files when we're cleaning, since in that
# case make will update the dependencies just before erasing them.
#
# The dependency files contain makefile rules that tell make what files
# depend on what. This means that if any file that this relies on it
# changed, things get rebuilt, which is a Good Thing.
#
ifneq ($(MAKECMDGOALS),clean)
-include $(DFILES)
endif

#
# Include the proper makefile depending on the project type.
#

ifeq "$(PROJECT_TYPE)" "lib"


#
# These rules make sure that the following get built in order to satisfy things:
#   a) Dependency information
#   b) Objects for this project
#   c) The directory libraries are stored in
#   d) The actual library we're creating
#
install:: $(DFILES) $(LOCAL_OBJS) $(LIBDIR)/.x $(LIB) 


#
# Clean rule for libraries. Gets rid of:
#   a) Dependency information
#   b) Objects for this project
#   c) The library itself
#
# The special scripts after the removal do the following:
#   a) If the library directory is empty after the binary is removed, then remove the bin
#      directory.
#   b) Same as A, but for the intermediate output directory.
#
clean::
	$(SHOWCOMP)rm -f $(DFILES) $(LOCAL_OBJS) $(LIB) 
	@rm -f $(LIBDIR)/.x
	@rm -f $(OBJDIR)/.x
	$(shell if [ -d $(OBJDIR) ] ; then rmdir --ignore-fail-on-non-empty -p $(OBJDIR); fi)
	$(shell if [ -d $(LIBDIR) ] ; then rmdir --ignore-fail-on-non-empty $(LIBDIR); fi)


else 

ifeq "$(PROJECT_TYPE)" "bin"


#
# These rules make sure that the following get built in order to satisfy things:
#   a) Dependency information
#   b) Objects for this project
#   c) The directory binaries are stored in
#   d) The actual binary we're creating
#   e) Any files that are being copies
install:: $(DFILES) $(LOCAL_OBJS) $(BINDIR)/.x $(BINDIR)/$(INTERNALNAME) $(COPYFILE_OUTPUT)


#
# Clean rule for binaries. Gets rid of:
#   a) Dependency information
#   b) Objects for this project
#   c) The binary itself
#
# The special scripts after the removal do the following:
#   a) If the binary directory is empty after the binary is removed, then remove the bin
#      directory.
#   b) Same as A, but for the intermediate output directory.
#   c) For each file that we would have copied, if the file that was copied is unchanged,
#      then remove it. This makes sure that if a file was copied, the modifications won't
#      be lost when the make clean happens.
#   d) If there is a file in the current directory with the same name as the binary, and
#      it's a symlink, remove it.
#
clean::
	$(SHOWCOMP)-rm -rf $(DFILES) $(LOCAL_OBJS) $(BINDIR)/$(INTERNALNAME)
	@rm -f $(BINDIR)/.x 
	@rm -f $(OBJDIR)/.x
	$(shell if [ -d $(OBJDIR) ] ; then rmdir --ignore-fail-on-non-empty -p $(OBJDIR); fi)
	$(shell if [ -d $(BINDIR) ] ; then rmdir -p --ignore-fail-on-non-empty $(BINDIR); fi)
	$(foreach FILE, $(COPYFILE_OUTPUT),$(shell cmp -s $(FILE) $(notdir $(FILE)); if [ $$? -eq 0 ] ; then rm -f $(FILE); fi))
	$(shell if [ -L $(INTERNALNAME) ] ; then rm -f $(INTERNALNAME); fi)

#
# Release rule for binaries strips the debug information from the created binary.
#
release::
	@strip $(BINDIR)/$(INTERNALNAME)

else

ifeq "$(PROJECT_TYPE)" "copy"

#
# This is an empty target type so that makefiles that don't need
# to compile anything can be made.
#

else

#
# This happens if the project type is unknown.
#
install::
	$(error "unknown project type")
	@echo project type is $(PROJECT_TYPE)

endif # copy
endif # bin
endif # lib


#
# Rule definitions. Tell make which rules don't actually create files,
# then blow away the list of implicit rules for suffixes we care about,
# so we can supply out own.
#
.PHONY : install clean default
.SUFFIXES :
.SUFFIXES : .c .cpp .m .d 
SUFFIXES := %.c %.cpp %.m %.d 
%:: RCS/%,v
%:: RCS/%
%:: %,v
%:: s.%
%:: SCCS/s.%
%.c: %.w %.ch


#
# Rule to make libraries. These work slightly different depending on 
# which type of library we're creating.
#
ifeq "$(STATIC)" "YES"
$(LIB): $(LOCAL_OBJS)
	$(ECHOSTAT) Linking $(LIB)
	$(SHOWCOMP)$(AR) $(ARFLAGS) $(LIB) $^
	$(COLOUR_START)
	$(SHOWCOMP)$(RANLIB) $(LIB) $(SUPPRESS_GNUSTEP_WARNING)
	$(COLOUR_END)
else 
$(LIB): $(LOCAL_OBJS)
	$(ECHOSTAT) Linking $(LIB)
	$(COLOUR_START)
	$(SHOWCOMP)$(CC) -shared -Wl,-soname,$(notdir $(LIB)) -o $(LIB) $^ -lc $(LINK_FLAGS) $(INCLUDED_LIBS) $(SUPPRESS_GNUSTEP_WARNING)
	$(COLOUR_END)
endif


#
# If there were any dynamic libraries specified that we're linking to,
# then add some extra options to the link line. In particular, it tells the
# executable that it can find it's libraries in a directory named lib under
# the directory that it was stored in (in addition to other system-defined
# locations). This makes sure that this binary will dynamically link to the
# library in the same source tree as it
#
ifneq "$(strip $(DLIBS))" ""
LINK_FLAGS+= -Wl,-rpath,../lib
endif


#
# If there are any static libraries specified for us, they are static libraries
# in our own source tree, so our binaries should be dependant on them, such that
# we will relink of they change (but we won't build them if they don't exist).	
#
ifneq "$(strip $(SLIBS))" ""
ALMOST_LIBS= $(addprefix $(LIBDIR)/lib, $(SLIBS))
DEPENDANT_LIBS= $(addsuffix .a, $(ALMOST_LIBS))
endif
	
#
# Rules to make binaries :
#   a) Be dependant on any static libraries that are in the project tree.
#   b) For any dynamic libraries we link to in our project tree, give an
#      rpath so that the dynamic linker finds files in the same source tree
#      as us before it looks elsewhere.
$(BINDIR)/$(INTERNALNAME):  $(OBJDIR)/.x $(DEPENDANT_LIBS) $(LOCAL_OBJS) $(BINDIR)/.x
	$(ECHOSTAT) Linking $(INTERNALNAME)
	$(COLOUR_START)
	$(SHOWCOMP)$(CC) $(ALL_CFLAGS) $(LINK_FLAGS) $(LOCAL_OBJS) $(TARGET_LINK_FLAGS) $(OBJC_LINK_START) $(INCLUDED_LIBS) $(OBJC_LINK_STOP) $(TARGET_LINK_POST) -o $@ $(SUPPRESS_GNUSTEP_WARNING)
	$(COLOUR_END)
	$(shell if [ ! -e $(INTERNALNAME) -o -L $(INTERNALNAME) ] ; then ln -sf $(BINDIR)/$(INTERNALNAME) $(INTERNALNAME); fi)


#
# This rule creates the directory libraries are stored in.
#
$(LIBDIR)/.x:
	@echo "Making $(LIBDIR)"
	@-mkdir -p $(LIBDIR)
	@-touch $(LIBDIR)/.x


#
# This rule creates the directory libraries are stored in.
#
$(BINDIR)/.x:
	@echo "Making $(BINDIR)"
	@-mkdir -p $(BINDIR)
	@-touch $(BINDIR)/.x


#
# This rule makes the directory that object files and dependencies
# are stored in.
#
$(OBJDIR)/.x:
	@echo "Making $(OBJDIR)"
	@-mkdir -p $(OBJDIR)
	@-touch $(OBJDIR)/.x


#
# This rule copies a file to it's destination
#
$(OUTPUT_DIR)/%: %
	$(ECHOSTAT) Copying $<
	$(SHOWCOMP)cp $< $(OUTPUT_DIR)


#
# This rule makes a directory
#
$(DIRECTORIES):
	$(ECHOSTAT) Making $(DIRECTORIES)
	$(SHOWCOMP) mkdir -p $(DIRECTORIES)


#
# The rules for compiling sources to object files.
# There is one for each type of source.
#
$(OBJDIR)/%.o: %.c $(OBJDIR)/.x
	$(ECHOSTAT) Compiling $<
	$(COLOUR_START)
	$(SHOWCOMP)$(CC) -c -D__C__ -DVERSION=$(VERSION) -DREVISION=$(REVISION) -DBUILD_HOST=$(BUILD_HOST) $(ALL_CFLAGS) -o $@ $<
	$(COLOUR_END)
	
$(OBJDIR)/%.o: %.cpp $(OBJDIR)/.x
	$(ECHOSTAT) Compiling $<
	$(COLOUR_START)
	$(SHOWCOMP)$(CXX) -c -D__CPP__ -DVERSION=$(VERSION) -DREVISION=$(REVISION) -DBUILD_HOST=$(BUILD_HOST) $(ALL_CPPFLAGS) -o $@ $<
	$(COLOUR_END)

$(OBJDIR)/%.o: %.m $(OBJDIR)/.x
	$(ECHOSTAT) Compiling $<
	$(COLOUR_START)
	$(SHOWCOMP)$(CC) -c -D__OBJC__ -DVERSION=$(VERSION) -DREVISION=$(REVISION) -DBUILD_HOST=$(BUILD_HOST) $(ALL_MFLAGS) -o $@ $<
	$(COLOUR_END)


#
# Rules to make dependencies, again one for each language.
#
$(OBJDIR)/%.d: %.c $(OBJDIR)/.x
	@echo "Making Dependency for $<"
	$(COLOUR_START)
	@$(SHELL) -ec '$(CC) -M -w $(ALL_CFLAGS) $< \
	| sed '\''s!\($*\)\.o[ :]*!$(OBJDIR)/\1.o $@ : !g'\'' > $@; \
	[ -s $@ ] || rm -f $@'
	$(COLOUR_END)

$(OBJDIR)/%.d: %.cpp $(OBJDIR)/.x
	@echo "Making Dependency for $<"
	$(COLOUR_START)
	@$(SHELL) -ec '$(CXX) -M -w $(ALL_CPPFLAGS) $< \
	| sed '\''s!\($*\)\.o[ :]*!$(OBJDIR)/\1.o $@ : !g'\'' > $@; \
	[ -s $@ ] || rm -f $@'
	$(COLOUR_END)

$(OBJDIR)/%.d: %.m $(OBJDIR)/.x
	@echo "Making Dependency for $<"
	$(COLOUR_START)
	@$(SHELL) -ec '$(CC) -M -w $(ALL_MFLAGS) $< \
	| sed '\''s!\($*\)\.o[ :]*!$(OBJDIR)/\1.o $@ : !g'\'' > $@; \
	[ -s $@ ] || rm -f $@'
	$(COLOUR_END)

