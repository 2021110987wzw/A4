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

#include "optimizer/OMRSimplifierHelpers.hpp"
#include "optimizer/Simplifier.hpp"

#include <limits.h>
#include <math.h>
#include "codegen/CodeGenerator.hpp"
#include "codegen/TreeEvaluator.hpp"
#include "compile/Compilation.hpp"
#include "env/CompilerEnv.hpp"
#include "env/IO.hpp"
#include "env/jittypes.h"
#include "il/AliasSetInterface.hpp"
#include "il/Block.hpp"
#include "il/DataTypes.hpp"
#include "il/ILOpCodes.hpp"
#include "il/LabelSymbol.hpp"
#include "il/Node.hpp"
#include "il/Node_inlines.hpp"
#include "il/SymbolReference.hpp"
#include "il/TreeTop.hpp"
#include "il/TreeTop_inlines.hpp"
#include "infra/Bit.hpp"
#include "infra/BitVector.hpp"
#include "infra/Cfg.hpp"
#include "optimizer/Optimization_inlines.hpp"
#include "optimizer/Optimizer.hpp"
#include "optimizer/Structure.hpp"
#include "optimizer/UseDefInfo.hpp"
#include "optimizer/ValueNumberInfo.hpp"

/*
 * Local helper functions
 */

//---------------------------------------------------------------------
// Determine the ordinal value associated with a node. This is used to determine
// the order in which children of a commutative node should be placed.
//
static intptr_t ordinalValue(TR::Node * node)
   {
   if (node->getOpCode().hasSymbolReference())
      return (intptr_t)node->getSymbolReference()->getReferenceNumber();
   return (intptr_t)node->getOpCodeValue();
   }

//---------------------------------------------------------------------
// Determine the order in which the children of a commutative node should be
// placed. The children of commutative nodes are ordered in a well-defined order
// so that commoning can be done on them.
// Return true if the children should be swapped.
//
static bool shouldSwapChildren(TR::Node * firstChild, TR::Node * secondChild)
   {
   intptr_t firstOrdinal = ordinalValue(firstChild);
   intptr_t secondOrdinal = ordinalValue(secondChild);
   if (firstOrdinal < secondOrdinal)
      return false;
   if (firstOrdinal > secondOrdinal)
      return true;
   if (firstChild->getNumChildren() == 0)
      return false;
   if (secondChild->getNumChildren() == 0)
      return true;
   return shouldSwapChildren(firstChild->getFirstChild(), secondChild->getFirstChild());
   }

//---------------------------------------------------------------------
// Common routine to swap the children of the node
//
static bool swapChildren(TR::Node * node, TR::Simplifier * s)
   {
   // NB: although one might be tempted, do not turn this child swap dumpOptDetails into a performTransformation
   // because we require that a tree with a constant node have that constant as the second child, unless both
   // are constant, so it is not conditional transformation, it is required
   dumpOptDetails(s->comp(), "%sSwap children of node [%s] %s\n", s->optDetailString(), node->getName(s->getDebug()), node->getOpCode().getName());
   node->swapChildren();
   return true;
   }

//**************************************
// Constant folding perform
//
bool performTransformationSimplifier(TR::Node * node, TR::Simplifier * s)
   {
   return performTransformation(s->comp(), "%sConstant folding node [%s] %s", s->optDetailString(), node->getName(s->getDebug()), node->getOpCode().getName());
   }

void setIsHighWordZero(TR::Node * node, TR::Simplifier * s)
   {
   if (((int32_t)(node->getLongIntHigh() & 0xffffffff) == (int32_t)0) &&
       ((int64_t)node->getLongInt() >= (int64_t)0))
      node->setIsHighWordZero(true);
   else
      node->setIsHighWordZero(false);
   }

TR::Node *_gotoSimplifier(TR::Node * node, TR::Block * block, TR::TreeTop* curTree, TR::Optimization * s)
   {
   if (branchToFollowingBlock(node, block, s->comp()))
      {
      if (node->getNumChildren() > 0)
         {
         TR_ASSERT(node->getFirstChild()->getOpCodeValue() == TR::GlRegDeps, "Expecting TR::GlRegDeps");

         // has GlRegDeps(after GRA), can be removed only if BBExit has exactly the same GlRegDeps
         if (block->getExit()->getNode()->getNumChildren() == 0)
            return node;
         if (!s->optimizer()->areNodesEquivalent(node->getFirstChild(), block->getExit()->getNode()->getFirstChild()))
            return node;
         }

      // Branch to the immediately following block. The goto can be removed
      //
      if (performTransformation(s->comp(), "%sRemoving goto [" POINTER_PRINTF_FORMAT "] to following block\n", s->optDetailString(), node))
         {
         s->removeNode(node, curTree);
         return NULL;
         }
      }
   return node;
   }

void foldIntConstant(TR::Node * node, int32_t value, TR::Simplifier * s, bool anchorChildrenP)
   {
   if (!performTransformationSimplifier(node, s)) return;

   if (anchorChildrenP) s->anchorChildren(node, s->_curTree);

   if (node->getOpCode().isRef())
      {
      static const char *jiagblah = feGetEnv("TR_JIAGTypeAssumes");
      if(jiagblah)
        TR_ASSERT(0, "Should never foldIntConstant on a reference Node!\n");
      s->prepareToReplaceNode(node, TR::aconst);
      node->setAddress(value);
      dumpOptDetails(s->comp(), " to %s %d\n", node->getOpCode().getName(), node->getAddress());

      }
   else
      {
      s->prepareToReplaceNode(node, TR::iconst);
      node->setInt(value);
      dumpOptDetails(s->comp(), " to %s %d\n", node->getOpCode().getName(), node->getInt());
      }
   }

void foldUIntConstant(TR::Node * node, uint32_t value, TR::Simplifier * s, bool anchorChildrenP)
   {
   if (!performTransformationSimplifier(node, s)) return;

   if (anchorChildrenP) s->anchorChildren(node, s->_curTree);

   s->prepareToReplaceNode(node, TR::iconst);
   node->setUnsignedInt(value);
   dumpOptDetails(s->comp(), " to %s %d\n", node->getOpCode().getName(), node->getInt());
   }

void foldLongIntConstant(TR::Node * node, int64_t value, TR::Simplifier * s, bool anchorChildrenP)
   {
   if (!performTransformationSimplifier(node, s)) return;

   if (anchorChildrenP) s->anchorChildren(node, s->_curTree);

   s->prepareToReplaceNode(node, node->getOpCode().isRef() ? TR::aconst : TR::lconst);

   if (node->getOpCode().isRef())
      node->setAddress((uintptr_t)value);
   else
      node->setLongInt(value);

   if (!node->getOpCode().isRef())
      setIsHighWordZero(node, s);

   dumpOptDetails(s->comp(), " to %s", node->getOpCode().getName());
   if (node->getLongIntHigh() == 0)
      dumpOptDetails(s->comp(), " 0x%x\n", node->getLongIntLow());
   else
      dumpOptDetails(s->comp(), " 0x%x%08x\n", node->getLongIntHigh(), node->getLongIntLow());
   }

void foldFloatConstant(TR::Node * node, float value, TR::Simplifier * s)
   {
   if (performTransformationSimplifier(node, s))
      {
      s->prepareToReplaceNode(node, TR::fconst);
      node->setFloat(value);
      dumpOptDetails(s->comp(), " to %s %f\n", node->getOpCode().getName(), node->getFloat());
      }
   }

void foldDoubleConstant(TR::Node * node, double value, TR::Simplifier * s)
   {
   if (performTransformationSimplifier(node, s))
      {
      s->prepareToReplaceNode(node, TR::dconst);
      node->setDouble(value);
      dumpOptDetails(s->comp(), " to %s %f\n", node->getOpCode().getName(), node->getDouble());
      }
   }

void foldByteConstant(TR::Node * node, int8_t value, TR::Simplifier * s, bool anchorChildrenP)
   {
   if (!performTransformationSimplifier(node, s)) return;

   if (anchorChildrenP) s->anchorChildren(node, s->_curTree);

   s->prepareToReplaceNode(node, TR::bconst);
   node->setByte(value);
   dumpOptDetails(s->comp(), " to %s %d\n", node->getOpCode().getName(), node->getByte());

   }

void foldShortIntConstant(TR::Node * node, int16_t value, TR::Simplifier * s, bool anchorChildrenP)
   {
   if (!performTransformationSimplifier(node, s))
      return;

   if (anchorChildrenP) s->anchorChildren(node, s->_curTree);

   s->prepareToReplaceNode(node, TR::sconst);
   node->setShortInt(value);
   dumpOptDetails(s->comp(), " to %s %d\n", node->getOpCode().getName(), node->getShortInt());
   }

void foldUByteConstant(TR::Node * node, uint8_t value, TR::Simplifier * s, bool anchorChildrenP)
   {
   if (!performTransformationSimplifier(node, s)) return;

   if (anchorChildrenP) s->anchorChildren(node, s->_curTree);

   s->prepareToReplaceNode(node, TR::bconst);
   node->setUnsignedByte(value);
   dumpOptDetails(s->comp(), " to %s %d\n", node->getOpCode().getName(), node->getUnsignedByte());
   }

void foldCharConstant(TR::Node * node, uint16_t value, TR::Simplifier * s, bool anchorChildrenP)
   {
   if (!performTransformationSimplifier(node, s)) return;

   if (anchorChildrenP) s->anchorChildren(node, s->_curTree);

   s->prepareToReplaceNode(node, TR::sconst);
   node->setConst<uint16_t>(value);
   dumpOptDetails(s->comp(), " to %s %d\n", node->getOpCode().getName(), node->getConst<uint16_t>());
   }

bool swapChildren(TR::Node * node, TR::Node * & firstChild, TR::Node * & secondChild, TR::Simplifier * s)
   {
   if (swapChildren(node, s))
      {
      firstChild = secondChild;
      secondChild = node->getSecondChild();
      return true;
      }
   return false;
   }

bool isExprInvariant(TR_RegionStructure *region, TR::Node *node)
   {
   if (node->getOpCode().isLoadConst())
      return true;

   if (region)
      {
      return region->isExprInvariant(node);
      }
   else
      return false;
   }

//**************************************
// Normalize a commutative binary tree
//
// If the first child is a constant but the second isn't, swap them.
// Also order the children in a well-defined order for better commoning
//
void orderChildren(TR::Node * node, TR::Node * & firstChild, TR::Node * & secondChild, TR::Simplifier * s)
   {
   TR_RegionStructure * region;

   if (!secondChild->getOpCode().isLoadConst() &&
       firstChild->getOpCode().isLoadConst())
      {
      swapChildren(node, firstChild, secondChild, s);
      }
   // R2:
   else if ((region = s->containingStructure()) &&
            !isExprInvariant(region, secondChild) &&
            isExprInvariant(region, firstChild))
      {
      if (performTransformation(s->comp(), "%sApplied reassociation rule 2 to node 0x%p\n", s->optDetailString(), node))
         swapChildren(node, firstChild, secondChild, s);
      }
   // R2:
   else if ((region = s->containingStructure()) &&
            isExprInvariant(region, secondChild) &&
            !isExprInvariant(region, firstChild))
      {
      // do nothing
      }
   else if (!secondChild->getOpCode().isLoadConst() &&
            shouldSwapChildren(firstChild, secondChild))
      {
      if (performTransformation(s->comp(), "%sOrdering children of node 0x%p\n", s->optDetailString(), node))
      swapChildren(node, firstChild, secondChild, s);
      }
   }

TR::Node *foldRedundantAND(TR::Node * node, TR::ILOpCodes andOpCode, TR::ILOpCodes constOpCode, int64_t andVal, TR::Simplifier * s)
   {
   TR::Node * andChild  = node->getFirstChild();

   if (andChild->getOpCodeValue() != andOpCode)
      return 0;

   TR::Node * lhsChild   = andChild->getFirstChild();
   TR::Node * constChild = andChild->getSecondChild();

   int64_t val;
   if (constChild->getOpCodeValue() == constOpCode)
      {
      switch(constOpCode)
         {
         case TR::sconst:
            val = constChild->getShortInt(); break;
         case TR::iconst:
            val = constChild->getInt(); break;
         case TR::lconst:
            val = constChild->getLongInt(); break;
         default:
            val = 0;
         }
      }
   else
      return 0;

   if (((val & andVal) == andVal) && (andChild->getReferenceCount() == 1) &&
    performTransformation(s->comp(), "%sFolding redundant AND node [%s] and its children [%s, %s]\n",
                           s->optDetailString(), node->getName(s->getDebug()), lhsChild->getName(s->getDebug()), constChild->getName(s->getDebug())))
      {
      TR::Node::recreate(andChild, andChild->getFirstChild()->getOpCodeValue());
      node->setAndIncChild(0, andChild->getFirstChild());
      s->prepareToStopUsingNode(andChild, s->_curTree);
      andChild->recursivelyDecReferenceCount();
      return node;
      }

   return 0;
   }

/** \brief
 *     Attempts to fold a logical and operation whose first child is a widening operation and whose second child is a
 *     constant node such that no bits in the constant node could ever overlap with the bits in the widened value.
 *
 *  \param simplifier
 *     The simplifier instance used to simplify the trees.
 *
 *  \param node
 *     The node which to attempt to fold.
 *
 *  \return
 *     The new folded node if a transformation was performed; NULL otherwise.
 */
TR::Node* tryFoldAndWidened(TR::Simplifier* simplifier, TR::Node* node)
   {
   if (node->getOpCode().isAnd())
      {
      TR::Node* rhsNode = node->getChild(1);

      if (rhsNode->getOpCode().isLoadConst())
         {
         TR::Node* lhsNode = node->getChild(0);

         // Look for zero extensions or known non-negative sign extensions
         if (lhsNode->getOpCode().isZeroExtension() || (lhsNode->getOpCode().isSignExtension() && lhsNode->isNonNegative()))
            {
            TR::Node* extensionValueNode = lhsNode->getChild(0);

            // Sanity check that this is indeed an extension
            TR_ASSERT(node->getSize() > extensionValueNode->getSize(), "Extended value datatype size must be smaller than the parent's datatype size.");

            // Produce a mask of 1 bits of the same width as the value being extended
            int64_t mask = (1ll << (8ll * static_cast<int64_t> (extensionValueNode->getSize()))) - 1ll;

            if ((rhsNode->getConstValue() & mask) == 0)
               {
               if (performTransformation(simplifier->comp(), "%sConstant folding widened and node [%p] to zero\n", simplifier->optDetailString(), node))
                  {
                  simplifier->anchorNode(extensionValueNode, simplifier->_curTree);

                  // This call will recreate the node as well
                  simplifier->prepareToReplaceNode(node, TR::ILOpCode::constOpCode(node->getDataType()));

                  node->setConstValue(0);

                  return node;
                  }
               }
            }
         }
      }

   return NULL;
   }

//---------------------------------------------------------------------
// Common routine to see if a branch is going immediately to the following block
//
bool branchToFollowingBlock(TR::Node * node, TR::Block * block, TR::Compilation *comp)
   {
   if (node->getBranchDestination() != block->getExit()->getNextTreeTop())
      return false;

   // If this is an extended basic block there may be real nodes after the
   // conditional branch. In this case the conditional branch must remain.
   //
   TR::TreeTop * treeTop = block->getLastRealTreeTop();
   if (treeTop->getNode() != node)
      return false;
   return true;
   }

// If the first child is a constant but the second isn't, swap them.
//
void makeConstantTheRightChild(TR::Node * node, TR::Node * & firstChild, TR::Node * & secondChild, TR::Simplifier * s)
   {
   if (firstChild->getOpCode().isLoadConst() &&
       !secondChild->getOpCode().isLoadConst())
      {
      swapChildren(node, firstChild, secondChild, s);
      }
   }

void makeConstantTheRightChildAndSetOpcode(TR::Node * node, TR::Node * & firstChild, TR::Node * & secondChild, TR::Simplifier * s)
   {
   if (firstChild->getOpCode().isLoadConst() &&
       !secondChild->getOpCode().isLoadConst())
      {
      TR_ASSERT(node->getOpCode().getOpCodeForSwapChildren() != TR::BadILOp,
             "cannot swap children of irreversible op");
      if (swapChildren(node, firstChild, secondChild, s))
         TR::Node::recreate(node, node->getOpCode().getOpCodeForSwapChildren());
      }
   }

// replaces an existing child whilst maintaining the ordering information from
// the original, returns the new child
TR::Node *replaceChild(int32_t childIndex, TR::Node* node, TR::Node* newChild, TR::Simplifier* s)
   {
   TR::Node* oldChild = node->getChild(childIndex);

   s->anchorOrderDependentNodesInSubtree(oldChild, newChild, s->_curTree);
   node->setAndIncChild(childIndex, newChild);
   oldChild->recursivelyDecReferenceCount();
   return newChild;
   }

TR::Node *postWalkLowerTreeSimplifier(TR::TreeTop *tt, TR::Node *node, TR::Block *block, TR::Simplifier * s)
   {
   TR::TreeTop * newTree = s->comp()->cg()->lowerTree(node, tt);
   if (newTree != s->_curTree)
      s->_curTree = newTree = newTree->getPrevTreeTop(); // set it to the previous treetop so simplier will next walk the new tree

   return node;
   }


void foldFloatConstantEmulate(TR::Node * node, uint32_t value, TR::Simplifier * s)
   {
   TR_ASSERT(false,"foldFloatConstantEmulate not implemented\n");
   return ;
   }

void foldDoubleConstantEmulate(TR::Node * node, uint64_t value, TR::Simplifier * s)
   {
   TR_ASSERT(false,"foldDoubleConstantEmulate not implemented\n");
   return ;
   }


//---------------------------------------------------------------------
// Check for special values
//

bool isNaNFloat(TR::Node * node)
   {
   if (!node->getOpCode().isLoadConst())
      return false;
   uint32_t value = (uint32_t)node->getFloatBits();
   return ((value >= FLOAT_NAN_1_LOW && value <= FLOAT_NAN_1_HIGH) ||
           (value >= FLOAT_NAN_2_LOW && value <= FLOAT_NAN_2_HIGH));
   }

bool isNaNDouble(TR::Node * node)
   {
   if (!node->getOpCode().isLoadConst())
      return false;
   uint64_t value = (uint64_t)node->getLongInt();
   return IN_DOUBLE_NAN_1_RANGE(value) || IN_DOUBLE_NAN_2_RANGE(value);
   }

bool isNZFloatPowerOfTwo(float value)
   {
   // return true if the float is a non-zero power of two
   union {
      float f;
      int32_t i;
   } u;
   int32_t float_exp, float_frac;
   u.f = value;
   float_exp = (u.i >> 23) & 0xff;
   float_frac = u.i & 0x7fffff;
   if (float_exp != 0 && float_exp != 0xff && float_frac == 0)
      return true;
   return false;
   }

bool isNZDoublePowerOfTwo(double value)
   {
   // return true if the double is a non-zero power of two
   union {
      double d;
      int64_t i;
   } u;
   int64_t double_exp, double_frac;
   u.d = value;
   double_exp = (u.i >> 52) & 0x7ff;
   double_frac = u.i & CONSTANT64(0xfffffffffffff);
   if (double_exp != 0 && double_exp != 0x7ff && double_frac == 0)
      return true;
   return false;
   }

// NOTE: This function only (and should only) decodes opcodes found in conversionMap table!!!
bool decodeConversionOpcode(TR::ILOpCode op, TR::DataType nodeDataType, TR::DataType &sourceDataType, TR::DataType &targetDataType)
   {
   if (!op.isConversion())
      {
      return false;
      }
   else
      {
      targetDataType = nodeDataType;
      TR::ILOpCodes opValue = op.getOpCodeValue();
      for (int i = 0; i < TR::NumAllTypes; i++)
          {
          if (TR::NumOMRTypes == i)
             {
             continue;
             }
          sourceDataType = (TR::DataTypes)i;
          if (opValue == TR::ILOpCode::getProperConversion(sourceDataType, targetDataType, false /*!wantZeroExtension*/))
             {
             return true;
             }
          }
      return false;
      }
   }

int32_t floatToInt(float value, bool roundUp)
   {
   int32_t pattern = *(int32_t *)&value;
   int32_t result;

   if ((pattern & 0x7f800000)==0x7f800000 && (pattern & 0x007fffff)!=0)
      result = 0;   // This is a NaN value
   else if (value <= TR::getMinSigned<TR::Int32>())
      result = static_cast<int32_t>(TR::getMinSigned<TR::Int32>());
   else if (value >= TR::getMaxSigned<TR::Int32>())
      result = static_cast<int32_t>(TR::getMaxSigned<TR::Int32>());
   else
      {
      if (roundUp)
         {
         if (value > 0)
            value += 0.5;
         else
            value -= 0.5;
         }
      result = (int32_t)value;
      }
   return result;
   }

int32_t doubleToInt(double value, bool roundUp)
   {
   int64_t pattern = *(int64_t *)&value;
   int32_t result;
   if ((pattern & DOUBLE_ORDER(CONSTANT64(0x7ff0000000000000))) == DOUBLE_ORDER(CONSTANT64(0x7ff0000000000000)) &&
       (pattern & DOUBLE_ORDER(CONSTANT64(0x000fffffffffffff))) != 0)
      result = 0;   // This is a NaN value
   else if (value <= TR::getMinSigned<TR::Int32>())
      result = static_cast<int32_t>(TR::getMinSigned<TR::Int32>());
   else if (value >= TR::getMaxSigned<TR::Int32>())
      result = static_cast<int32_t>(TR::getMaxSigned<TR::Int32>());
   else
      {
      if (roundUp)
         {
         if (value > 0)
            value += 0.5;
         else
            value -= 0.5;
         }

      result = (int32_t)value;
      }
   return result;
   }

void removePaddingNode(TR::Node *node, TR::Simplifier *s)
   {
   TR::Node *paddingNode = NULL;
   if (paddingNode)
      {
      if (!paddingNode->safeToDoRecursiveDecrement())
         s->anchorNode(paddingNode, s->_curTree);
      paddingNode->recursivelyDecReferenceCount();
      int32_t oldNumChildren = node->getNumChildren();
      node->setNumChildren(oldNumChildren-1);
      dumpOptDetails(s->comp(),"remove paddingNode %s (0x%p) from %s (0x%p) and dec numChildren %d->%d\n",
         paddingNode->getOpCode().getName(),paddingNode,node->getOpCode().getName(),node,oldNumChildren,oldNumChildren-1);
      }
   }

// the usual behaviour is to remove the padding node except in cases where the caller is already dealing with this
void stopUsingSingleNode(TR::Node *node, bool removePadding, TR::Simplifier *s)
   {
   TR_ASSERT(node->getReferenceCount() == 1, "stopUsingSingleNode only valid for nodes with a referenceCount of 1 and not %d\n",
         node->getReferenceCount());
   if (removePadding)
      removePaddingNode(node, s);
   if (node->getReferenceCount() <= 1)
      {
      if (s->optimizer()->prepareForNodeRemoval(node, /* deferInvalidatingUseDefInfo = */ true))
         s->_invalidateUseDefInfo = true;
      s->_alteredBlock = true;
      }
   node->decReferenceCount();
   if (node->getReferenceCount() > 0)
      node->setVisitCount(0);
   }

TR::TreeTop *findTreeTop(TR::Node * callNode, TR::Block * block)
   {
   // Walk the extended block to find this node - has to be child of store or tree-top
   TR::Block * b = block->startOfExtendedBlock();
   if (!b)
      return NULL;
   do
      {
      for (TR::TreeTop* tt = b->getEntry(); tt != b->getExit(); tt = tt->getNextRealTreeTop())
         {
         if (tt->getNode()->getNumChildren() == 1 && tt->getNode()->getFirstChild() == callNode)
            {
            return tt;
            }
         }
      b = b->getNextBlock();
      } while (b && b->isExtensionOfPreviousBlock());
   return NULL;
   }

TR::Node *removeIfToFollowingBlock(TR::Node * node, TR::Block * block, TR::Simplifier * s)
   {
   if (branchToFollowingBlock(node, block, s->comp()))
      {
      // Branch to the immediately following block. The branch can be removed
      //
      if (performTransformation(s->comp(), "%sRemoving %s [" POINTER_PRINTF_FORMAT "] to following block\n", s->optDetailString(), node->getOpCode().getName(), node))
         {
         s->prepareToStopUsingNode(node, s->_curTree);
         node->recursivelyDecReferenceCount();
         return NULL;
         }
      }
   return node;
   }

void normalizeShiftAmount(TR::Node * node, int32_t normalizationConstant, TR::Simplifier * s)
   {
   if (s->comp()->cg()->needsNormalizationBeforeShifts() &&
       !node->isNormalizedShift())
      {
      TR::Node * secondChild = node->getSecondChild();
      //
      // Some platforms like IA32 obey Java semantics for shifts even if the
      // shift amount is greater than 31. However other platforms like PPC need
      // to normalize the shift amount to range (0, 31) before shifting in order
      // to obey Java semantics. This can be captured in the IL and commoning/hoisting
      // can be done (look at Compressor.compress).
      //
      if ((secondChild->getOpCodeValue() != TR::iconst) &&
          ((secondChild->getOpCodeValue() != TR::iand) ||
           (secondChild->getSecondChild()->getOpCodeValue() != TR::iconst) ||
           (secondChild->getSecondChild()->getInt() != normalizationConstant)))
         {
         if (performTransformation(s->comp(), "%sPlatform specific normalization of shift node [%s]\n", s->optDetailString(), node->getName(s->getDebug())))
            {
            //
            // Not normalized yet
            //
            TR::Node * secondChild = node->getSecondChild();
            TR::Node * normalizedNode = TR::Node::create(TR::iand, 2, secondChild, TR::Node::create(secondChild, TR::iconst, 0, normalizationConstant));
            secondChild->recursivelyDecReferenceCount();
            node->setAndIncChild(1, normalizedNode);
            node->setNormalizedShift(true);
            s->_alteredBlock = true;
            }
         }
      }
   }
