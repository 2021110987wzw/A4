/*******************************************************************************
 * Copyright IBM Corp. and others 2000
 *
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse Public License 2.0 which accompanies this
 * distribution and is available at http://eclipse.org/legal/epl-2.0
 * or the Apache License, Version 2.0 which accompanies this distribution
 * and is available at https://www.apache.org/licenses/LICENSE-2.0.
 *
 * This Source Code may also be made available under the following Secondary
 * Licenses when the conditions for such availability set forth in the
 * Eclipse Public License, v. 2.0 are satisfied: GNU General Public License,
 * version 2 with the GNU Classpath Exception [1] and GNU General Public
 * License, version 2 with the OpenJDK Assembly Exception [2].
 *
 * [1] https://www.gnu.org/software/classpath/license.html
 * [2] https://openjdk.org/legal/assembly-exception.html
 *
 * SPDX-License-Identifier: EPL-2.0 OR Apache-2.0 OR GPL-2.0-only WITH Classpath-exception-2.0 OR GPL-2.0-only WITH OpenJDK-assembly-exception-1.0
 *******************************************************************************/

#if defined(OMR_OS_WINDOWS)
#undef BYTE
#include "windows.h"
#define PATH_MAX MAXPATHLEN
#else
#include <dlfcn.h>
#endif /* defined(OMR_OS_WINDOWS) */
#include <errno.h>
#include "OMRTestEnv.hpp"
#include "runtime/Runtime.hpp"

extern "C" bool initializeTestJit(TR_RuntimeHelper *helperIDs, void **helperAddresses, int32_t numHelpers, char *options);
extern "C" void shutdownJit();

void
TestCompiler::OMRTestEnv::SetUp()
   {
   initialize("-Xjit");
   }

void
TestCompiler::OMRTestEnv::TearDown()
   {
   shutdown();
   }

void
TestCompiler::OMRTestEnv::initialize(char *options)
   {
   initializeTestJit(0, 0, 0, options);
   }

void
TestCompiler::OMRTestEnv::shutdown()
   {
   shutdownJit();
   }
