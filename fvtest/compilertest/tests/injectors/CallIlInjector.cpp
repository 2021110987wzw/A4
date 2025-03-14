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

#include "compile/Compilation.hpp"
#include "env/FrontEnd.hpp"
#include "compile/ResolvedMethod.hpp"
#include "ilgen/TypeDictionary.hpp"
#include "tests/OpCodesTest.hpp"
#include "tests/injectors/CallIlInjector.hpp"

namespace TestCompiler
{

bool
CallIlInjector::injectIL()
   {
   if (!isOpCodeSupported())
      return false;
   OpCodesTest *test = static_cast<OpCodesTest *>(_test);
   createBlocks(1);
   // Block 2: blocks(0)
   // return function()
   TR::ResolvedMethod *resolvedCompilee = test->resolvedMethod(_dataType);
   returnValue(callFunction(resolvedCompilee, _types->PrimitiveType(_dataType), 1, parm(1)));

   return true;
   }

} // namespace TestCompiler
