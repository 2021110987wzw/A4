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

#ifndef RVHELPERCALLSNIPPET_INCL
#define RVHELPERCALLSNIPPET_INCL

#include "codegen/Snippet.hpp"

#include <stdint.h>
#include "il/SymbolReference.hpp"

namespace TR { class CodeGenerator; }
namespace TR { class LabelSymbol; }
namespace TR { class Node; }

namespace TR {

class RVHelperCallSnippet : public TR::Snippet
   {
   TR::SymbolReference      *_destination;
   TR::LabelSymbol          *_restartLabel;

   public:

   /**
    * @brief Constructor
    */
   RVHelperCallSnippet(TR::CodeGenerator    *cg,
                          TR::Node             *node,
                          TR::LabelSymbol      *snippetlab,
                          TR::SymbolReference  *helper,
                          TR::LabelSymbol      *restartLabel=NULL)
      : TR::Snippet(cg, node, snippetlab, helper->canCauseGC()),
        _destination(helper),
        _restartLabel(restartLabel)
      {
      }

   /**
    * @brief Answers the Snippet kind
    * @return Snippet kind
    */
   virtual Kind getKind() { return IsHelperCall; }

   /**
    * @brief Answers the destination
    * @return destination
    */
   TR::SymbolReference *getDestination() { return _destination; }
   /**
    * @brief Sets the destination
    * @return destination
    */
   TR::SymbolReference *setDestination(TR::SymbolReference *s) { return (_destination = s); }

   /**
    * @brief Answers the restart label
    * @return restart label
    */
   TR::LabelSymbol *getRestartLabel() { return _restartLabel; }
   /**
    * @brief Sets the restart label
    * @return restart label
    */
   TR::LabelSymbol *setRestartLabel(TR::LabelSymbol *l) { return (_restartLabel=l); }

   /**
    * @brief Emits the Snippet body
    * @return instruction cursor
    */
   virtual uint8_t *emitSnippetBody();

   /**
    * @brief Answers the Snippet length
    * @return Snippet length
    */
   virtual uint32_t getLength(int32_t estimatedSnippetStart);
   };

}

#endif
