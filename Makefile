###############################################################################
#
# This Makefile recurses into each project subdirectory for the targets that
# the build system understands, so that a make in this directory will build
# everything. 
#
###############################################################################

# 
# For each target we support, cd into all subdirectories of us that we want
# built, and recursively call ourselves.
#
install clean release::
	@cd core    && $(MAKE) $@
	@cd vm      && $(MAKE) $@
#	@cd project && $(MAKE) $@
