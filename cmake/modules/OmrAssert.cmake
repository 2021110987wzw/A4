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
#############################################################################

# Include once
if(OMR_ASSERT_)
	return()
endif()
set(OMR_ASSERT_ 1)

# omr_assert([<mode>] TEST <condition>... [MESSAGE <message>])
# <mode> is one of FATAL_ERROR (default), WARNING, STATUS
function(omr_assert)
	cmake_parse_arguments(omr_assert "STATUS;WARNING;SEND_ERROR;FATAL_ERROR" "MESSAGE" "TEST" ${ARGN})

	if(omr_assert_FATAL_ERROR)
		set(mode FATAL_ERROR)
	elseif(omr_assert_SEND_ERROR)
		set(mode SEND_ERROR)
	elseif(omr_assert_WARNING)
		set(mode WARNING)
	elseif(omr_assert_STATUS)
		set(mode STATUS)
	else()
		set(mode FATAL_ERROR)
	endif()

	if(DEFINED omr_assert_UNPARSED_ARGUMENTS)
		message(FATAL_ERROR "omr_assert called with extra arguments")
	endif()

	if(NOT DEFINED omr_assert_TEST)
		message(FATAL_ERROR "omr_assert called without test condition")
	endif()

	if(NOT DEFINED omr_assert_MESSAGE)
		string(REPLACE ";" " " omr_assert_MESSAGE "${omr_assert_TEST}")
	endif()

	if(NOT(${omr_assert_TEST}))
		message(${mode} "Assertion failed: ${omr_assert_MESSAGE}")
	endif()
endfunction()
