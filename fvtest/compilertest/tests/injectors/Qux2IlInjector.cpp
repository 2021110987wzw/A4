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
#include "tests/injectors/Qux2IlInjector.hpp"

namespace TestCompiler
{

bool
Qux2IlInjector::injectIL()
   {
   createBlocks(1);
   // Block 2: blocks(0)
   // int32_t i = parameter;
   // i = i * 2;
   // return i;
   TR::SymbolReference *newIndexSymRef = newTemp(Int32);
   storeToTemp(newIndexSymRef, intParameter());
   storeToTemp(newIndexSymRef, createWithoutSymRef(TR::imul, 2, loadTemp(newIndexSymRef), iconst(2)));
   returnValue(loadTemp(newIndexSymRef));

   return true;
   }

} // namespace TestCompiler
