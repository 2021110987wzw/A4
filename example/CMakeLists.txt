###############################################################################
# Copyright IBM Corp. and others 2017
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

include(OmrAssert)
include(OmrTest)

omr_assert(TEST OMR_EXAMPLE)

omr_add_library(omr_example_base INTERFACE)

target_include_directories(omr_example_base
	INTERFACE
		glue
)

target_link_libraries(omr_example_base
	INTERFACE
		omr_base
)

# The GC example shows how to use OMR's GC component
if(OMR_GC)
	omr_add_executable(gcexample
		gcmain.cpp
	)

	target_link_libraries(gcexample
		omr_main_function
		pugixml
		omrcore
		omrvmstartup
		${OMR_GC_LIB}
		${OMR_PORT_LIB}
	)

	omr_add_test(NAME gcexample COMMAND $<TARGET_FILE:gcexample>)
endif(OMR_GC)

add_subdirectory(glue)
