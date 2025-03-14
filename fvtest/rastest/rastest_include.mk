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

OBJECTS += main_function$(OBJEXT)
vpath main_function.cpp $(top_srcdir)/util/main_function

MODULE_INCLUDES += $(OMR_GTEST_INCLUDES) ../util
MODULE_INCLUDES += $(OMR_IPATH)
MODULE_CXXFLAGS += $(OMR_GTEST_CXXFLAGS)

MODULE_STATIC_LIBS += \
  j9prtstatic \
  j9thrstatic \
  omrutil \
  j9avl \
  j9hashtable \
  j9omr \
  j9pool \
  omrtrace \
  omrGtest \
  testutil \
  omrgcbase \
  omrgcstructs \
  omrgcstats \
  omrgcstandard \
  omrgcstartup \
  omrgcverbose \
  omrgcverbosehandlerstandard \
  omrvmstartup \
  j9hookstatic \
  omrglue

ifeq (gcc,$(OMR_TOOLCHAIN))
  MODULE_SHARED_LIBS += stdc++
endif
ifeq (linux,$(OMR_HOST_OS))
  MODULE_SHARED_LIBS += rt pthread
endif
ifeq (osx,$(OMR_HOST_OS))
  MODULE_SHARED_LIBS += iconv pthread
endif
ifeq (aix,$(OMR_HOST_OS))
  MODULE_SHARED_LIBS += iconv perfstat
endif
ifeq (win,$(OMR_HOST_OS))
  MODULE_SHARED_LIBS += ws2_32 # socket library
  MODULE_SHARED_LIBS += shell32 Iphlpapi psapi pdh
endif
