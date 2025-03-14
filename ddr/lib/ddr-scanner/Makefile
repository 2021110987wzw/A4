###############################################################################
# Copyright IBM Corp. and others 2015
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

top_srcdir = ../../..
include $(top_srcdir)/omrmakefiles/configure.mk

MODULE_NAME := ddr-scanner
ARTIFACT_TYPE := archive

MODULE_INCLUDES = \
  $(top_srcdir)/ddr/include \
  $(top_srcdir)/include_core

ifeq (gcc,$(OMR_TOOLCHAIN))
  MODULE_CXXFLAGS += -frtti -D__STDC_LIMIT_MACROS -std=c++0x
endif

ifeq (msvc,$(OMR_TOOLCHAIN))
  MODULE_CXXFLAGS += /EHsc
endif

OBJECTS = Scanner$(OBJEXT)

ifneq (,$(findstring pdb,$(DEBUG_FORMAT)))
  OBJECTS += pdb/PdbScanner$(OBJEXT)
  MODULE_STATIC_LIBS += diaguids.lib
endif

ifneq (,$(findstring dwo,$(DEBUG_FORMAT)))
  OBJECTS += dwarf/DwarfScanner$(OBJEXT)
endif

ifeq (osx,$(OMR_HOST_OS))
  OBJECTS += dwarf/DwarfParser$(OBJEXT) \
    dwarf/DwarfFunctions$(OBJEXT)
endif

ifeq (aix,$(OMR_HOST_OS))
  OBJECTS += dwarf/AixSymbolTableParser$(OBJEXT) \
    dwarf/DwarfFunctions$(OBJEXT)
endif

include $(top_srcdir)/omrmakefiles/rules.mk
