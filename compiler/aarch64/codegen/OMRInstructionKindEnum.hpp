/*******************************************************************************
 * Copyright IBM Corp. and others 2018
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

   IsNotExtended,
   IsImm,
      IsSynchronization,
      IsException,
   IsRelocatableImm,
   IsImmSym,
   IsLabel,
      IsConditionalBranch,
      IsCompareBranch,
      IsTestBitBranch,
      IsVirtualGuardNOP,
   IsRegBranch,
   IsAdmin,
   IsTrg1,
      IsTrg1Cond,
      IsTrg1Imm,
         IsTrg1ImmShifted,
         IsTrg1ImmSym,
      IsTrg1ZeroSrc1,
      IsTrg1ZeroImm,
      IsTrg1Src1,
         IsTrg1Src1Imm,
         IsTrg1Src2,
            IsCondTrg1Src2,
            IsTrg1Src2Imm,
            IsTrg1Src2Shifted,
            IsTrg1Src2Extended,
            IsTrg1Src2IndexedElement,
            IsTrg1Src2Zero,
            IsTrg1Src3,
      IsTrg1Mem,
         IsTrg1MemSrc1,
         IsTrg2Mem,
   IsMem,
      IsMemSrc1,
         IsMemSrc2,
      IsMemImm,
   IsSrc1,
      IsZeroSrc1Imm,
      IsSrc1ImmCond,
      IsSrc2,
         IsZeroSrc2,
         IsSrc2Cond
