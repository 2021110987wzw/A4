/*******************************************************************************
 * Copyright IBM Corp. and others 2019
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

/*
 * This file will be included within an enum.  Only comments and enumerator
 * definitions are permitted.
 */
      NoReg             = 0,

#define DECLARE_GPR(regname, abiname, encoding) \
      abiname           = NoReg + 1 + encoding, \
      regname           = NoReg + 1 + encoding,
#include "codegen/riscv-regs.h"
#undef DECLARE_GPR

#define DECLARE_FPR(regname, abiname, encoding) \
      abiname           = x31 + 1 + encoding, \
      regname           = x31 + 1 + encoding,
#include "codegen/riscv-regs.h"
#undef DECLARE_FPR
      FirstGPR          = x0,
      LastGPR           = x31,
      LastAssignableGPR = x31,

      FirstFPR          = f0,
      LastFPR           = f31,
      LastAssignableFPR = f31,
      SpilledReg        = f31 + 1,
      NumRegisters      = SpilledReg + 1,
