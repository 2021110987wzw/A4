/*******************************************************************************
 * Copyright IBM Corp. and others 2021
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

#ifndef OMR_Z_JITCODERX_OBJECTFORMAT_INCL
#define OMR_Z_JITCODERX_OBJECTFORMAT_INCL

/*
 * The following #define and typedef must appear before any #includes in this file
 */
#ifndef OMR_JITCODERX_OBJECTFORMAT_CONNECTOR
#define OMR_JITCODERX_OBJECTFORMAT_CONNECTOR
namespace OMR { namespace Z { class JitCodeRXObjectFormat; } }
namespace OMR { typedef OMR::Z::JitCodeRXObjectFormat JitCodeRXObjectFormatConnector; }
#else
#error OMR::Z::JitCodeRXObjectFormat expected to be a primary connector, but a OMR connector is already defined
#endif

#include "compiler/objectfmt/OMRJitCodeRXObjectFormat.hpp"

namespace TR { class Instruction; }
namespace TR { class FunctionCallData; }

namespace OMR
{

namespace Z
{

class OMR_EXTENSIBLE JitCodeRXObjectFormat : public OMR::JitCodeRXObjectFormat
   {
public:

   virtual TR::Instruction *emitFunctionCall(TR::FunctionCallData &data);

   virtual uint8_t *encodeFunctionCall(TR::FunctionCallData &data);

   virtual int32_t estimateBinaryLength()
      {
      return 8;
      }

   virtual uint8_t* printEncodedFunctionCall(TR::FILE *pOutFile, TR::FunctionCallData &data);

   struct ccGlobalFunctionData
      {
      uintptr_t address;
      };

   };

}

}

#endif