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

#ifndef TR_OPTIMIZATIONDATA_INCL
#define TR_OPTIMIZATIONDATA_INCL

#include "compile/Compilation.hpp"
#include "env/TRMemory.hpp"

namespace TR
{

class OptimizationData
	{
	public:

	static void *operator new(size_t size, TR::Allocator a)
           { return a.allocate(size); }
        static void  operator delete(void *ptr, size_t size)
           { ((OptimizationData*)ptr)->allocator().deallocate(ptr, size); } /* t->allocator() must return the same allocator as used for new */
        static void operator delete(void *p, TR::Allocator a) {}

        /* Virtual destructor is necessary for the above delete operator to work
         * See "Modern C++ Design" section 4.7
         */
        virtual ~OptimizationData() {}

        OptimizationData(TR::Compilation *comp) : _comp(comp) {}

	TR::Compilation *comp() { return _comp; }
	TR::Allocator allocator() { return comp()->allocator(); }

	private:
	TR::Compilation *_comp;
	};

}

#endif
