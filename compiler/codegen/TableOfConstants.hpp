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

#ifndef OMR_TABLEOFCONSTANTS_INCL
#define OMR_TABLEOFCONSTANTS_INCL

/*
 * This class is used to manage the mapping/assigning of table of constant (TOC)
 * slots on the platforms which choose to support it.  Each code generator should
 * extend this class to realize exactly how the TOC is managed (e.g., linearly
 * allocated or hashed).
 */

#include <stdint.h>

namespace OMR
{

class TableOfConstants
   {

   public:

   TableOfConstants(uint32_t size)
      : _sizeInBytes(size) {}

   uint32_t getTOCSize() {return _sizeInBytes;}
   void setTOCSize(uint32_t s) {_sizeInBytes = s;}

   private:

   uint32_t _sizeInBytes;
   };

}

using OMR::TableOfConstants;

#endif
