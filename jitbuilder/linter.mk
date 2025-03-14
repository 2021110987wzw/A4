###############################################################################
# Copyright IBM Corp. and others 2016
#
# This program and the accompanying materials are made available under
# the terms of the Eclipse Public License 2.0 which accompanies this
# distribution and is available at http://eclipse.org/legal/epl-2.0
# or the Apache License, Version 2.0 which accompanies this distribution
# and is available at https://www.apache.org/licenses/LICENSE-2.0.
#
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the
# Eclipse Public License, v. 2.0 are satisfied: GNU General Public License,
# version 2 with the GNU Classpath Exception [1] and GNU General Public
# License, version 2 with the OpenJDK Assembly Exception [2].
#
# [1] https://www.gnu.org/software/classpath/license.html
# [2] https://openjdk.org/legal/assembly-exception.html
#
# SPDX-License-Identifier: EPL-2.0 OR Apache-2.0 OR GPL-2.0-only WITH Classpath-exception-2.0 OR GPL-2.0-only WITH OpenJDK-assembly-exception-1.0
###############################################################################

# To lint:
#
# PLATFORM=foo-bar-clang make -f linter.mk
#

.PHONY: linter
linter:

#default paths, unless overriden
export LLVM_CONFIG?=llvm-config
export CC_PATH?=clang
export CXX_PATH?=clang++

#
# First setup some important paths
# Personally, I feel it's best to default to out-of-tree build but who knows, there may be
# differing opinions on that.
#
JIT_SRCBASE?=../
JIT_OBJBASE?=../objs/jitbuilder_$(BUILD_CONFIG)
JIT_DLL_DIR?=$(JIT_OBJBASE)

#
# Windows users will likely use backslashes, but Make tends to not like that so much
#
FIXED_SRCBASE=$(subst \,/,$(JIT_SRCBASE))
FIXED_OBJBASE=$(subst \,/,$(JIT_OBJBASE))
FIXED_DLL_DIR=$(subst \,/,$(JIT_DLL_DIR))

#
# Default to debug build to catch linting errors in code guarded by
# `#ifdef DEBUG`
#
BUILD_CONFIG?=debug

#
# This is where we setup our component dirs
# Note these are all relative to JIT_SRCBASE and JIT_OBJBASE
# It just makes sense since source and build dirs may be in different places
# in the filesystem :)
#
JIT_OMR_DIRTY_DIR?=compiler
JIT_PRODUCT_DIR?=jitbuilder

#
# Dirs used internally by the makefiles
#
JIT_MAKE_DIR?=$(FIXED_SRCBASE)/jitbuilder/build
JIT_SCRIPT_DIR?=$(FIXED_SRCBASE)/tools/compiler/scripts

#
# First we set a bunch of tokens about the platform that the rest of the
# makefile will use as conditionals
#
include $(JIT_MAKE_DIR)/platform/common.mk

#
# Now we include the names of all the files that will go into building the JIT
# Will automatically include files needed from HOST and TARGET platform
#
include $(JIT_MAKE_DIR)/files/common.mk

#
# Now we configure all the tooling we will use to build the files
#
# There is quite a bit of shared stuff, but the overwhelming majority of this
# is toolchain-dependent.
#
# That makes sense - You can't expect XLC and GCC to take the same arguments
#
include $(JIT_MAKE_DIR)/toolcfg/common.mk


#
# Add OMRChecker targets
#
# This likely ought to be using the OMRChecker fragment that
# exists in that repo, but in the mean time, this is fine.
#

OMRCHECKER_DIR?=$(JIT_SRCBASE)/tools/compiler/OMRChecker

OMRCHECKER_OBJECT=$(OMRCHECKER_DIR)/OMRChecker.so

OMRCHECKER_LDFLAGS=`$(LLVM_CONFIG) --ldflags`
OMRCHECKER_CXXFLAGS=`$(LLVM_CONFIG) --cxxflags`

$(OMRCHECKER_OBJECT):
	cd $(OMRCHECKER_DIR); sh smartmake.sh

.PHONY: omrchecker omrchecker_test omrchecker_clean omrchecker_cleandll omrchecker_cleanall
omrchecker: $(OMRCHECKER_OBJECT)

omrchecker_test: $(OMRCHECKER_OBJECT)
	cd $(OMRCHECKER_DIR); make test

omrchecker_clean:
	cd $(OMRCHECKER_DIR); make clean

omrchecker_cleandll:
	cd $(OMRCHECKER_DIR); make cleandll

omrchecker_cleanall:
	cd $(OMRCHECKER_DIR); make cleanall


# The core linter bits.
#
# linter is the default target, and we construct a pre-req for each
#  .cpp file.
#
linter: omrchecker

# It seems that different versions of clang either do or do not define these,
# however, it appears that it's not possible to check for definitions
# using macros.
#
# Given that the purpose of linting is categorically not to produce a
# functional binary, and to ease our progress right now, we choose to exclude
# the problematic builtins, wiping out their definitions with nothing.
#
# A longer term answer might involve autoconf detection in the omr project,
# however, that doesn't fix all of our problems, as we are typically not
# reconfiguring for clang when running lint (though, perhaps this is an early
# indicator that we really should be.
EXCLUDED_DEFINES='-D__sync()=' '-D__lwsync()=' '-D__isync()='

# The clang invocation magic line.
LINTER_EXTRA=-Xclang -load -Xclang $(OMRCHECKER_OBJECT) -Xclang -add-plugin -Xclang omr-checker
LINTER_FLAGS=-std=c++0x -w -fsyntax-only $(EXCLUDED_DEFINES) -ferror-limit=0 $(LINTER_FLAGS_EXTRA)

define DEF_RULE.linter
.PHONY: $(1).linted

$(1).linted: $(1) omrchecker
	$$(CXX_CMD) $(LINTER_FLAGS) $(LINTER_EXTRA)  $$(patsubst %,-D%,$$(CXX_DEFINES)) $$(patsubst %,-I'%',$$(CXX_INCLUDES)) -o $$@ -c $$<
linter: $(1).linted

endef # DEF_RULE.linter

RULE.linter=$(eval $(DEF_RULE.linter))

# The list of sources.
JIT_CPP_FILES=$(filter %.cpp,$(JIT_PRODUCT_SOURCE_FILES) $(JIT_PRODUCT_BACKEND_SOURCES))

# Construct lint dependencies.
$(foreach SRCFILE,$(JIT_CPP_FILES),\
   $(call RULE.linter,$(FIXED_SRCBASE)/$(SRCFILE)))
