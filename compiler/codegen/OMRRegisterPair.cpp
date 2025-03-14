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

#include "codegen/RegisterPair.hpp"

#include <stddef.h>
#include <stdint.h>
#include "codegen/CodeGenerator.hpp"
#include "codegen/LiveRegister.hpp"
#include "codegen/Register.hpp"
#include "codegen/RegisterPair.hpp"
#include "infra/List.hpp"

void OMR::RegisterPair::block()
   {
   _lowOrder->block();
   _highOrder->block();
   }

void OMR::RegisterPair::unblock()
   {
   _lowOrder->unblock();
   _highOrder->unblock();
   }

bool OMR::RegisterPair::usesRegister(TR::Register* reg)
   {
   if (self() == reg || _lowOrder == reg || _highOrder == reg )
      {
      return true;
      }
   else
      {
      return false;
      }
   }

TR::Register *
OMR::RegisterPair::getLowOrder()
   {
   return _lowOrder;
   }

TR::Register *
OMR::RegisterPair::setLowOrder(TR::Register *lo, TR::CodeGenerator *cg)
   {
   if (!lo->isLive() && cg->getLiveRegisters(lo->getKind())!=NULL)
      cg->getLiveRegisters(lo->getKind())->addRegister(lo);

   return (_lowOrder = lo);
   }

TR::Register *
OMR::RegisterPair::getHighOrder()
   {
   return _highOrder;
   }

TR::Register *
OMR::RegisterPair::setHighOrder(TR::Register *ho, TR::CodeGenerator *cg)
   {
   if (!ho->isLive() && cg->getLiveRegisters(ho->getKind())!=NULL)
      cg->getLiveRegisters(ho->getKind())->addRegister(ho);

   return (_highOrder = ho);
   }

TR::Register *
OMR::RegisterPair::getRegister()
   {
   return NULL;
   }

TR::RegisterPair *
OMR::RegisterPair::getRegisterPair()
   {
   return self();
   }

TR::RegisterPair *
OMR::RegisterPair::self()
   {
   return static_cast<TR::RegisterPair*>(this);
   }
