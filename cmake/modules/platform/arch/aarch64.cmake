###############################################################################
# Copyright IBM Corp. and others 2018
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
#############################################################################

list(APPEND OMR_PLATFORM_DEFINITIONS
	-DAARCH64
	-DJ9AARCH64
	-DFIXUP_UNALIGNED
	-march=armv8-a+simd
)

# Apple clang does not support the following option.
if(NOT (CMAKE_C_COMPILER_ID MATCHES "^(Apple)?Clang$"))
	list(APPEND OMR_PLATFORM_DEFINITIONS
		-Wno-unused-but-set-variable
	)
endif()

set(TR_HOST_ARCH aarch64)
set(TR_HOST_BITS 64)

list(APPEND TR_COMPILE_DEFINITIONS TR_HOST_ARM64 TR_TARGET_ARM64 TR_HOST_64BIT TR_TARGET_64BIT BITVECTOR_64BIT)
