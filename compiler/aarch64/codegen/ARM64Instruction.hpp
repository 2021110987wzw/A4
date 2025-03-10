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

#ifndef ARM64INSTRUCTION_INCL
#define ARM64INSTRUCTION_INCL

#include <stddef.h>
#include <stdint.h>

#include "codegen/ARM64ConditionCode.hpp"
#include "codegen/ARM64ShiftCode.hpp"
#include "codegen/CodeGenerator.hpp"
#include "codegen/Instruction.hpp"
#include "codegen/MemoryReference.hpp"
#include "codegen/RegisterDependency.hpp"
#include "codegen/UnresolvedDataSnippet.hpp"
#include "il/LabelSymbol.hpp"
#include "infra/Assert.hpp"

class TR_VirtualGuardSite;
namespace TR { class SymbolReference; }

#define ARM64_INSTRUCTION_LENGTH 4

/*
 * Prefetch type used in PRFM instructions
 */
enum class ARM64PrefetchType
   {
   LOAD        = 0,
   INSTRUCTION = 1,
   STORE       = 2
   };

/*
 * Prefetch target used in PRFM instructions
 */
enum class ARM64PrefetchTarget
   {
   L1 = 0,
   L2 = 1,
   L3 = 2
   };

/*
 * Prefetch policy used in PRFM instructions
 */
enum class ARM64PrefetchPolicy
   {
   KEEP = 0,
   STRM = 1
   };

/*
 * @brief returns Prefetch operation from specified type, target, and policy.
 * @param[in] type : prefetch type
 * @param[in] target : prefetch target
 * @param[in] policy : prefetch policy
 * @return prefetch operation
 */
inline uint32_t toPrefetchOp(ARM64PrefetchType type, ARM64PrefetchTarget target, ARM64PrefetchPolicy policy)
   {
   return (static_cast<uint32_t>(type) << 3) | (static_cast<uint32_t>(target) << 1) | static_cast<uint32_t>(policy);
   }

/*
 * @brief Answers if the signed integer value can be placed in 7-bit field
 * @param[in] intValue : signed integer value
 * @return true if the value can be placed in 7-bit field, false otherwise
 */
inline bool constantIsImm7(int32_t intValue)
   {
   return (-64 <= intValue && intValue < 64);
   }

/*
 * @brief Answers if the signed integer value can be placed in 9-bit field
 * @param[in] intValue : signed integer value
 * @return true if the value can be placed in 9-bit field, false otherwise
 */
inline bool constantIsImm9(int64_t intValue)
   {
   return (-256 <= intValue && intValue < 256);
   }

/*
 * @brief Answers if the unsigned integer value can be encoded in a 4-bit field
 * @param[in] intValue : unsigned integer value
 * @return true if the value can be encoded in a 4-bit field, false otherwise
 */
inline bool constantIsUnsignedImm4(uint64_t intValue)
   {
   return (intValue < (1<<4));  // 16
   }

/*
 * @brief Answers if the unsigned integer value can be encoded in a 5-bit field
 * @param[in] intValue : unsigned integer value
 * @return true if the value can be encoded in a 5-bit field, false otherwise
 */
inline bool constantIsUnsignedImm5(uint64_t intValue)
   {
   return (intValue < (1<<5));  // 32
   }

/*
 * @brief Answers if the unsigned integer value can be encoded in a 12-bit field
 * @param[in] intValue : unsigned integer value
 * @return true if the value can be encoded in a 12-bit field, false otherwise
 */
inline bool constantIsUnsignedImm12(uint64_t intValue)
   {
   return (intValue < (1<<12));  // 4096
   }

/*
 * @brief Answers if the unsigned integer value can be encoded in a 12-bit field with 12 bits shift
 * @param[in] intValue : unsigned integer value
 * @return true if the value can be encoded in a 12-bit field with 12 bits shift, false otherwise
 */
inline bool constantIsUnsignedImm12Shifted(uint64_t intValue)
   {
   return ((intValue & (~(static_cast<uint64_t>(0xfff000)))) == 0);
   }

/*
 * @brief Answers if the signed integer value can be placed in a 16-bit field
 * @param[in] intValue : signed integer value
 * @return true if the value can be placed in 16-bit field, false otherwise
 */
inline bool constantIsSignedImm16(intptr_t intValue)
   {
   return (-0x8000 <= intValue && intValue < 0x8000);
   }

/*
 * @brief Answers if the unsigned integer value can be encoded in a 16-bit field
 * @param[in] intValue : unsigned integer value
 * @return true if the value can be encoded in a 16-bit field, false otherwise
 */
inline bool constantIsUnsignedImm16(uint64_t intValue)
   {
   return (intValue < (1<<16));  // 65536
   }

/*
 * @brief Answers if the signed integer value can be placed in 21-bit field
 * @param[in] intValue : signed integer value
 * @return true if the value can be placed in 21-bit field, false otherwise
 */
inline bool constantIsSignedImm21(intptr_t intValue)
   {
   return (-0x100000 <= intValue && intValue < 0x100000);
   }

/*
 * @brief Answers if the signed integer value can be placed in 28-bit field
 * @param[in] intValue : signed integer value
 * @return true if the value can be placed in 28-bit field, false otherwise
 */
inline bool constantIsSignedImm28(intptr_t intValue)
   {
   return (-0x8000000 <= intValue && intValue < 0x8000000);
   }

namespace TR
{

class ARM64ImmInstruction : public TR::Instruction
   {
   uint32_t _sourceImmediate;

public:

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] imm : immediate value
    * @param[in] cg : CodeGenerator
    */
   ARM64ImmInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, uint32_t imm, TR::CodeGenerator *cg)
      : TR::Instruction(op, node, cg), _sourceImmediate(imm)
      {
      }
   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] imm : immediate value
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64ImmInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, uint32_t imm,
                       TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : TR::Instruction(op, node, precedingInstruction, cg), _sourceImmediate(imm)
      {
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsImm; }

   /**
    * @brief Gets source immediate
    * @return source immediate
    */
   uint32_t getSourceImmediate() {return _sourceImmediate;}
   /**
    * @brief Sets source immediate
    * @param[in] si : immediate value
    * @return source immediate
    */
   uint32_t setSourceImmediate(uint32_t si) {return (_sourceImmediate = si);}

   /**
    * @brief Encodes the immediate value into the instruction
    * @param[in] instruction : instruction address
    */
   virtual void insertImmediateField(uint32_t *instruction)
      {
      // Write the 32-bit immediate with no masking or shifting.
      // Be aware this will overwrite the entire 32-bit instruction
      // with the immediate value.
      //
      *instruction = _sourceImmediate;
      }

   /**
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();
   };

class ARM64RelocatableImmInstruction : public TR::Instruction
   {
   uintptr_t _sourceImmediate;
   TR_ExternalRelocationTargetKind _reloKind;
   TR::SymbolReference *_symbolReference;

public:

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] imm : immediate value
    * @param[in] relocationKind : relocation kind
    * @param[in] cg : CodeGenerator
    */
   ARM64RelocatableImmInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, uintptr_t imm,
                       TR_ExternalRelocationTargetKind relocationKind, TR::CodeGenerator *cg)
      : TR::Instruction(op, node, cg),
        _sourceImmediate(imm), _reloKind(relocationKind), _symbolReference(NULL)
      {
      setNeedsAOTRelocation();
      }
   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] imm : immediate value
    * @param[in] relocationKind : relocation kind
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64RelocatableImmInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, uintptr_t imm,
                       TR_ExternalRelocationTargetKind relocationKind, TR::Instruction *precedingInstruction,
                       TR::CodeGenerator *cg)
      : TR::Instruction(op, node, precedingInstruction, cg),
        _sourceImmediate(imm), _reloKind(relocationKind), _symbolReference(NULL)
      {
      setNeedsAOTRelocation();
      }
   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] imm : immediate value
    * @param[in] relocationKind : relocation kind
    * @param[in] sr : symbol reference
    * @param[in] cg : CodeGenerator
    */
   ARM64RelocatableImmInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, uintptr_t imm,
                       TR_ExternalRelocationTargetKind relocationKind, TR::SymbolReference *sr,
                       TR::CodeGenerator *cg)
      : TR::Instruction(op, node, cg),
        _sourceImmediate(imm), _reloKind(relocationKind), _symbolReference(sr)
      {
      setNeedsAOTRelocation();
      }
   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] imm : immediate value
    * @param[in] relocationKind : relocation kind
    * @param[in] sr : symbol reference
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64RelocatableImmInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, uintptr_t imm,
                       TR_ExternalRelocationTargetKind relocationKind, TR::SymbolReference *sr,
                       TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : TR::Instruction(op, node, precedingInstruction, cg),
        _sourceImmediate(imm), _reloKind(relocationKind), _symbolReference(sr)
      {
      setNeedsAOTRelocation();
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsRelocatableImm; }

   /**
    * @brief Gets source immediate
    * @return source immediate
    */
   uintptr_t getSourceImmediate() {return _sourceImmediate;}
   /**
    * @brief Sets source immediate
    * @param[in] si : immediate value
    * @return source immediate
    */
   uintptr_t setSourceImmediate(intptr_t si) {return (_sourceImmediate = si);}

   /**
    * @brief Encodes the immediate value into the instruction
    * @param[in] instruction : instruction address
    */
   void insertImmediateField(uintptr_t *instruction)
      {
      *instruction = _sourceImmediate;
      }

   /**
    * @brief Sets relocation kind
    * @param[in] reloKind : relocation kind
    * @return relocation kind
    */
   TR_ExternalRelocationTargetKind setReloKind(TR_ExternalRelocationTargetKind reloKind) { return (_reloKind = reloKind); }
   /**
    * @brief Gets relocation kind
    * @return relocation kind
    */
   TR_ExternalRelocationTargetKind getReloKind() { return _reloKind; }

   /**
    * @brief Sets symbol reference
    * @param[in] sr : symbol reference
    * @return symbol reference
    */
   TR::SymbolReference *setSymbolReference(TR::SymbolReference *sr) { return (_symbolReference = sr); }
   /**
    * @brief Gets symbol reference
    * @return symbol reference
    */
   TR::SymbolReference *getSymbolReference() { return _symbolReference; }

   /**
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();

   /**
    * @brief Estimates binary length
    * @param[in] currentEstimate : current estimated length
    * @return estimated binary length
    */
   virtual int32_t estimateBinaryLength(int32_t currentEstimate);
   };

class ARM64ImmSymInstruction : public TR::Instruction
   {
   uintptr_t _addrImmediate;
   TR::SymbolReference *_symbolReference;
   TR::Snippet *_snippet;

   public:

   ARM64ImmSymInstruction(TR::InstOpCode::Mnemonic op,
                          TR::Node *node,
                          uintptr_t imm,
                          TR::RegisterDependencyConditions *cond,
                          TR::SymbolReference *sr,
                          TR::Snippet *s,
                          TR::CodeGenerator *cg)
      : TR::Instruction(op, node, cond, cg),
        _addrImmediate(imm), _symbolReference(sr), _snippet(s)
      {
      }

   ARM64ImmSymInstruction(TR::InstOpCode::Mnemonic op,
                          TR::Node *node,
                          uintptr_t imm,
                          TR::RegisterDependencyConditions *cond,
                          TR::SymbolReference *sr,
                          TR::Snippet *s,
                          TR::Instruction *precedingInstruction,
                          TR::CodeGenerator *cg)
      : TR::Instruction(op, node, cond, precedingInstruction, cg),
        _addrImmediate(imm), _symbolReference(sr), _snippet(s)
      {
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsImmSym; }

   /**
    * @brief Gets address immediate
    * @return address immediate
    */
   uintptr_t getAddrImmediate() { return _addrImmediate; }
   /**
    * @brief Sets address immediate
    * @param[in] imm : address immediate
    * @return address immediate
    */
   uintptr_t setAddrImmediate(uintptr_t imm) { return (_addrImmediate = imm); }

   /**
    * @brief Gets symbol reference
    * @return symbol reference
    */
   TR::SymbolReference *getSymbolReference() { return _symbolReference; }
   /**
    * @brief Sets symbol reference
    * @param[in] sr : symbol reference
    * @return symbol reference
    */
   TR::SymbolReference *setSymbolReference(TR::SymbolReference *sr)
      {
      return (_symbolReference = sr);
      }

   /**
    * @brief Gets call snippet
    * @return call snippet
    */
   TR::Snippet *getCallSnippet() { return _snippet;}
   /**
    * @brief Sets call snippet
    * @param[in] s : call snippet
    * @return call snippet
    */
   TR::Snippet *setCallSnippet(TR::Snippet *s) { return (_snippet = s); }

   virtual TR::Snippet *getSnippetForGC() {return _snippet;}

   /**
    * @brief Sets immediate field in binary encoding
    * @param[in] instruction : instruction cursor
    * @param[in] distance : branch distance
    */
   void insertImmediateField(uint32_t *instruction, int32_t distance)
      {
      TR_ASSERT((distance & 0x3) == 0, "branch distance is not aligned");
      *instruction |= ((distance >> 2) & 0x3ffffff); // imm26
      }

   /**
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();
   };

class ARM64LabelInstruction : public TR::Instruction
   {
   TR::LabelSymbol *_symbol;

   public:

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] sym : label symbol
    * @param[in] cg : CodeGenerator
    */
   ARM64LabelInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::LabelSymbol *sym, TR::CodeGenerator *cg)
      : TR::Instruction(op, node, cg), _symbol(sym)
      {
      if (sym != NULL && op == TR::InstOpCode::label)
         sym->setInstruction(this);
      }
   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] sym : label symbol
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64LabelInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::LabelSymbol *sym,
                         TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : TR::Instruction(op, node, precedingInstruction, cg), _symbol(sym)
      {
      if (sym!=NULL && op==TR::InstOpCode::label)
         sym->setInstruction(this);
      }
   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] sym : label symbol
    * @param[in] cond : register dependency condition
    * @param[in] cg : CodeGenerator
    */
   ARM64LabelInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::LabelSymbol *sym,
                         TR::RegisterDependencyConditions *cond, TR::CodeGenerator *cg)
      : TR::Instruction(op, node, cond, cg), _symbol(sym)
      {
      if (sym!=NULL && op==TR::InstOpCode::label)
         sym->setInstruction(this);
      }
   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] sym : label symbol
    * @param[in] cond : register dependency condition
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64LabelInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::LabelSymbol *sym,
                         TR::RegisterDependencyConditions *cond,
                         TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : TR::Instruction(op, node, cond, precedingInstruction, cg), _symbol(sym)
      {
      if (sym!=NULL && op==TR::InstOpCode::label)
         sym->setInstruction(this);
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsLabel; }

   /**
    * @brief Gets label symbol
    * @return label symbol
    */
   TR::LabelSymbol *getLabelSymbol() {return _symbol;}
   /**
    * @brief Sets label symbol
    * @param[in] sym : label symbol
    * @return label symbol
    */
   TR::LabelSymbol *setLabelSymbol(TR::LabelSymbol *sym)
      {
      return (_symbol = sym);
      }

   virtual TR::Snippet *getSnippetForGC() {return getLabelSymbol()->getSnippet();}

   /**
    * @brief Sets immediate field in binary encoding
    * @param[in] instruction : instruction cursor
    * @param[in] distance : branch distance
    */
   void insertImmediateField(uint32_t *instruction, int32_t distance)
      {
      TR_ASSERT((distance & 0x3) == 0, "branch distance is not aligned");
      *instruction |= ((distance >> 2) & 0x3ffffff); // imm26
      }

   /**
    * @brief Assigns registers
    * @param[in] kindToBeAssigned : register kind
    */
   virtual void assignRegisters(TR_RegisterKinds kindToBeAssigned);

   /**
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();

   /**
    * @brief Estimates binary length
    * @param[in] currentEstimate : current estimated length
    * @return estimated binary length
    */
   virtual int32_t estimateBinaryLength(int32_t currentEstimate);

   protected:

   /**
    * @brief Assigns registers for OutOfLineCodeSection
    *
    * @param[in] kindToBeAssigned : register kind
    */
   void assignRegistersForOutOfLineCodeSection(TR_RegisterKinds kindToBeAssigned);
   };

class ARM64ConditionalBranchInstruction : public ARM64LabelInstruction
   {
   int32_t _estimatedBinaryLocation;
   TR::ARM64ConditionCode _cc;

   public:

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] sym : label symbol
    * @param[in] cc : branch condition code
    * @param[in] cg : CodeGenerator
    */
   ARM64ConditionalBranchInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::LabelSymbol *sym,
                                     TR::ARM64ConditionCode cc,
                                     TR::CodeGenerator *cg)
      : ARM64LabelInstruction(op, node, sym, cg), _cc(cc),
        _estimatedBinaryLocation(0)
      {
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] sym : label symbol
    * @param[in] cc : branch condition code
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64ConditionalBranchInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::LabelSymbol *sym,
                                     TR::ARM64ConditionCode cc,
                                     TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : ARM64LabelInstruction(op, node, sym, precedingInstruction, cg), _cc(cc),
        _estimatedBinaryLocation(0)
      {
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] sym : label symbol
    * @param[in] cc : branch condition code
    * @param[in] cond : register dependency condition
    * @param[in] cg : CodeGenerator
    */
   ARM64ConditionalBranchInstruction(
                             TR::InstOpCode::Mnemonic op,
                             TR::Node *node,
                             TR::LabelSymbol *sym,
                             TR::ARM64ConditionCode cc,
                             TR::RegisterDependencyConditions *cond, TR::CodeGenerator *cg)
      : ARM64LabelInstruction(op, node, sym, cond, cg), _cc(cc),
        _estimatedBinaryLocation(0)
      {
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] sym : label symbol
    * @param[in] cc : branch condition code
    * @param[in] cond : register dependency condition
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64ConditionalBranchInstruction(
                             TR::InstOpCode::Mnemonic op,
                             TR::Node *node,
                             TR::LabelSymbol *sym,
                             TR::ARM64ConditionCode cc,
                             TR::RegisterDependencyConditions *cond,
                             TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : ARM64LabelInstruction(op, node, sym, cond, precedingInstruction, cg), _cc(cc),
        _estimatedBinaryLocation(0)
      {
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsConditionalBranch; }

   /**
    * @brief Gets estimated binary location
    * @return estimated binary location
    */
   int32_t getEstimatedBinaryLocation() {return _estimatedBinaryLocation;}
   /**
    * @brief Sets estimated binary location
    * @param[in] l : estimated binary location
    * @return estimated binary location
    */
   int32_t setEstimatedBinaryLocation(int32_t l) {return (_estimatedBinaryLocation = l);}

   /**
    * @brief Gets condition code
    * @return condition code
    */
   TR::ARM64ConditionCode getConditionCode() {return _cc;}
   /**
    * @brief Sets condition code
    * @param[in] cc : condition code
    * @return condition code
    */
   TR::ARM64ConditionCode setConditionCode(TR::ARM64ConditionCode cc) {return (_cc = cc);}

   /**
    * @brief Sets immediate field in binary encoding
    * @param[in] instruction : instruction cursor
    * @param[in] distance : branch distance
    */
   void insertImmediateField(uint32_t *instruction, int32_t distance)
      {
      TR_ASSERT((distance & 0x3) == 0, "branch distance is not aligned");
      *instruction |= ((distance >> 2) & 0x7ffff) << 5; // imm19
      }

   /**
    * @brief Sets condition code in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertConditionCodeField(uint32_t *instruction)
      {
      *instruction |= (_cc & 0xf);
      }

   /**
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();

   /**
    * @brief Estimates binary length
    * @param[in] currentEstimate : current estimated length
    * @return estimated binary length
    */
   virtual int32_t estimateBinaryLength(int32_t currentEstimate);
   };

class ARM64CompareBranchInstruction : public ARM64LabelInstruction
   {
   int32_t _estimatedBinaryLocation;
   TR::Register *_source1Register;

   public:

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] sreg : source register
    * @param[in] sym : label symbol
    * @param[in] cg : CodeGenerator
    */
   ARM64CompareBranchInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *sreg, TR::LabelSymbol *sym,
                                 TR::CodeGenerator *cg)
      : ARM64LabelInstruction(op, node, sym, cg), _source1Register(sreg),
        _estimatedBinaryLocation(0)
      {
      useRegister(sreg);
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] sreg : source register
    * @param[in] sym : label symbol
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64CompareBranchInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *sreg, TR::LabelSymbol *sym,
                                 TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : ARM64LabelInstruction(op, node, sym, precedingInstruction, cg), _source1Register(sreg),
        _estimatedBinaryLocation(0)
      {
      useRegister(sreg);
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] sreg : source register
    * @param[in] sym : label symbol
    * @param[in] cond : register dependency condition
    * @param[in] cg : CodeGenerator
    */
   ARM64CompareBranchInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *sreg, TR::LabelSymbol *sym,
                                 TR::RegisterDependencyConditions *cond, TR::CodeGenerator *cg)
      : ARM64LabelInstruction(op, node, sym, cond, cg), _source1Register(sreg),
        _estimatedBinaryLocation(0)
      {
      useRegister(sreg);
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] sreg : source register
    * @param[in] sym : label symbol
    * @param[in] cond : register dependency condition
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64CompareBranchInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *sreg, TR::LabelSymbol *sym,
                                 TR::RegisterDependencyConditions *cond, TR::Instruction *precedingInstruction,
                                 TR::CodeGenerator *cg)
      : ARM64LabelInstruction(op, node, sym, cond, precedingInstruction, cg), _source1Register(sreg),
        _estimatedBinaryLocation(0)
      {
      useRegister(sreg);
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsCompareBranch; }

   /**
    * @brief Gets estimated binary location
    * @return estimated binary location
    */
   int32_t getEstimatedBinaryLocation() {return _estimatedBinaryLocation;}
   /**
    * @brief Sets estimated binary location
    * @param[in] l : estimated binary location
    * @return estimated binary location
    */
   int32_t setEstimatedBinaryLocation(int32_t l) {return (_estimatedBinaryLocation = l);}

   /**
    * @brief Gets source register
    * @return source register
    */
   TR::Register *getSource1Register() {return _source1Register;}
   /**
    * @brief Sets source register
    * @param[in] sr : source register
    * @return source register
    */
   TR::Register *setSource1Register(TR::Register *sr) {return (_source1Register = sr);}

   /**
    * @brief Sets immediate field in binary encoding
    * @param[in] instruction : instruction cursor
    * @param[in] distance : branch distance
    */
   void insertImmediateField(uint32_t *instruction, int32_t distance)
      {
      TR_ASSERT((distance & 0x3) == 0, "branch distance is not aligned");
      *instruction |= ((distance >> 2) & 0x7ffff) << 5;
      }

   /**
    * @brief Sets source register in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertSource1Register(uint32_t *instruction)
      {
      TR::RealRegister *source1 = toRealRegister(_source1Register);
      source1->setRegisterFieldRD(instruction);
      }

   /**
    * @brief Answers whether this instruction references the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction references the virtual register
    */
   virtual bool refsRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction uses the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction uses the virtual register
    */
   virtual bool usesRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction defines the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction defines the virtual register
    */
   virtual bool defsRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction defines the given real register
    * @param[in] reg : real register
    * @return true when the instruction defines the real register
    */
   virtual bool defsRealRegister(TR::Register *reg);
   /**
    * @brief Assigns registers
    * @param[in] kindToBeAssigned : register kind
    */
   virtual void assignRegisters(TR_RegisterKinds kindToBeAssigned);

   /**
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();
   };

class ARM64TestBitBranchInstruction : public ARM64LabelInstruction
   {
   int32_t _estimatedBinaryLocation;
   TR::Register *_source1Register;
   uint32_t _bitpos;

   public:

   /*
    * @brief Constructor
    * @param[in] op     : instruction opcode
    * @param[in] node   : node
    * @param[in] sreg   : source register
    * @param[in] bitpos : bit position to test
    * @param[in] sym    : label symbol
    * @param[in] cg     : CodeGenerator
    */
   ARM64TestBitBranchInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *sreg, uint32_t bitpos, TR::LabelSymbol *sym,
                                 TR::CodeGenerator *cg)
      : ARM64LabelInstruction(op, node, sym, cg), _source1Register(sreg),
        _estimatedBinaryLocation(0), _bitpos(bitpos)
      {
      useRegister(sreg);
      }

   /*
    * @brief Constructor
    * @param[in] op                   : instruction opcode
    * @param[in] node                 : node
    * @param[in] sreg                 : source register
    * @param[in] bitpos               : bit position to test
    * @param[in] sym                  : label symbol
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg                   : CodeGenerator
    */
   ARM64TestBitBranchInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *sreg, uint32_t bitpos, TR::LabelSymbol *sym,
                                 TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : ARM64LabelInstruction(op, node, sym, precedingInstruction, cg), _source1Register(sreg),
        _estimatedBinaryLocation(0), _bitpos(bitpos)
      {
      useRegister(sreg);
      }

   /*
    * @brief Constructor
    * @param[in] op     : instruction opcode
    * @param[in] node   : node
    * @param[in] sreg   : source register
    * @param[in] bitpos : bit position to test
    * @param[in] sym    : label symbol
    * @param[in] cond   : register dependency condition
    * @param[in] cg     : CodeGenerator
    */
   ARM64TestBitBranchInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *sreg, uint32_t bitpos, TR::LabelSymbol *sym,
                                 TR::RegisterDependencyConditions *cond, TR::CodeGenerator *cg)
      : ARM64LabelInstruction(op, node, sym, cond, cg), _source1Register(sreg),
        _estimatedBinaryLocation(0), _bitpos(bitpos)
      {
      useRegister(sreg);
      }

   /*
    * @brief Constructor
    * @param[in] op                   : instruction opcode
    * @param[in] node                 : node
    * @param[in] sreg                 : source register
    * @param[in] bitpos               : bit position to test
    * @param[in] sym                  : label symbol
    * @param[in] cond                s : register dependency condition
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg                   : CodeGenerator
    */
   ARM64TestBitBranchInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *sreg, uint32_t bitpos, TR::LabelSymbol *sym,
                                 TR::RegisterDependencyConditions *cond, TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : ARM64LabelInstruction(op, node, sym, cond, precedingInstruction, cg), _source1Register(sreg),
        _estimatedBinaryLocation(0), _bitpos(bitpos)
      {
      useRegister(sreg);
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsTestBitBranch; }

   /**
    * @brief Gets bit position
    * @return bit position
    */
   uint32_t getBitPos() { return _bitpos; }

   /**
    * @brief Sets bit position
    * @param[in] bitpos : bit position
    * @return bit position
    */
   uint32_t setBitPos(uint32_t bitpos) { return (_bitpos = bitpos);}

   /**
    * @brief Gets estimated binary location
    * @return estimated binary location
    */
   int32_t getEstimatedBinaryLocation() {return _estimatedBinaryLocation;}
   /**
    * @brief Sets estimated binary location
    * @param[in] l : estimated binary location
    * @return estimated binary location
    */
   int32_t setEstimatedBinaryLocation(int32_t l) {return (_estimatedBinaryLocation = l);}

   /**
    * @brief Gets source register
    * @return source register
    */
   TR::Register *getSource1Register() {return _source1Register;}
   /**
    * @brief Sets source register
    * @param[in] sr : source register
    * @return source register
    */
   TR::Register *setSource1Register(TR::Register *sr) {return (_source1Register = sr);}
   /**
    * @brief Sets immediate field in binary encoding
    * @param[in] instruction : instruction cursor
    * @param[in] distance : branch distance
    */
   void insertImmediateField(uint32_t *instruction, int32_t distance)
      {
      TR_ASSERT_FATAL((distance & 0x3) == 0, "branch distance is not aligned");
      *instruction |= ((distance >> 2) & 0x3fff) << 5;
      }

   /**
    * @brief Sets source register in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertSource1Register(uint32_t *instruction)
      {
      TR::RealRegister *source1 = toRealRegister(_source1Register);
      source1->setRegisterFieldRD(instruction);
      }

   /**
    * @brief Sets bit position field in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertBitposField(uint32_t *instruction)
      {
      TR_ASSERT_FATAL(_bitpos <= 63, "bit position must be 0 to 63");
      *instruction |= (((_bitpos >> 5) & 1) << 31) | ((_bitpos & 0x1f) << 19);
      }

   /**
    * @brief Answers whether this instruction references the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction references the virtual register
    */
   virtual bool refsRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction uses the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction uses the virtual register
    */
   virtual bool usesRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction defines the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction defines the virtual register
    */
   virtual bool defsRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction defines the given real register
    * @param[in] reg : real register
    * @return true when the instruction defines the real register
    */
   virtual bool defsRealRegister(TR::Register *reg);
   /**
    * @brief Assigns registers
    * @param[in] kindToBeAssigned : register kind
    */
   virtual void assignRegisters(TR_RegisterKinds kindToBeAssigned);

   /**
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();
   };

class ARM64RegBranchInstruction : public TR::Instruction
   {
   TR::Register *_register;

   public:

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : branch target
    * @param[in] cg : CodeGenerator
    */
   ARM64RegBranchInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *treg, TR::CodeGenerator *cg)
      : TR::Instruction(op, node, cg), _register(treg)
      {
      useRegister(treg);
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : branch target
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64RegBranchInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *treg,
                             TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : TR::Instruction(op, node, precedingInstruction, cg), _register(treg)
      {
      useRegister(treg);
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : branch target
    * @param[in] cond : register dependency condition
    * @param[in] cg : CodeGenerator
    */
   ARM64RegBranchInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *treg,
                             TR::RegisterDependencyConditions *cond, TR::CodeGenerator *cg)
      : TR::Instruction(op, node, cond, cg), _register(treg)
      {
      useRegister(treg);
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : branch target
    * @param[in] cond : register dependency condition
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64RegBranchInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *treg,
                             TR::RegisterDependencyConditions *cond,
                             TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : TR::Instruction(op, node, cond, precedingInstruction, cg), _register(treg)
      {
      useRegister(treg);
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsRegBranch; }

   /**
    * @brief Gets target register
    * @return target register
    */
   TR::Register *getTargetRegister() {return _register;}
   /**
    * @brief Sets target register
    * @param[in] tr : target register
    * @return target register
    */
   TR::Register *setTargetRegister(TR::Register *tr) {return (_register = tr);}

   /**
    * @brief Sets target register in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertTargetRegister(uint32_t *instruction)
      {
      TR::RealRegister *target = toRealRegister(_register);
      target->setRegisterFieldRN(instruction);
      }

   /**
    * @brief Answers whether this instruction references the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction references the virtual register
    */
   virtual bool refsRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction uses the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction uses the virtual register
    */
   virtual bool usesRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction defines the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction defines the virtual register
    */
   virtual bool defsRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction defines the given real register
    * @param[in] reg : real register
    * @return true when the instruction defines the real register
    */
   virtual bool defsRealRegister(TR::Register *reg);
   /**
    * @brief Assigns registers
    * @param[in] kindToBeAssigned : register kind
    */
   virtual void assignRegisters(TR_RegisterKinds kindToBeAssigned);

   /**
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();
   };

class ARM64AdminInstruction : public TR::Instruction
   {
   TR::Node *_fenceNode;

   public:

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] fenceNode : fence node
    * @param[in] cg : CodeGenerator
    */
   ARM64AdminInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Node *fenceNode, TR::CodeGenerator *cg)
      : TR::Instruction(op, node, cg), _fenceNode(fenceNode)
      {
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] fenceNode : fence node
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64AdminInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Node *fenceNode,
                         TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : TR::Instruction(op, node, precedingInstruction, cg), _fenceNode(fenceNode)
      {
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] cond : register dependency conditions
    * @param[in] node : node
    * @param[in] fenceNode : fence node
    * @param[in] cg : CodeGenerator
    */
   ARM64AdminInstruction(TR::InstOpCode::Mnemonic op, TR::RegisterDependencyConditions *cond,
                         TR::Node *node, TR::Node *fenceNode, TR::CodeGenerator *cg)
      : TR::Instruction(op, node, cond, cg), _fenceNode(fenceNode)
      {
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] cond : register dependency conditions
    * @param[in] node : node
    * @param[in] fenceNode : fence node
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64AdminInstruction(TR::InstOpCode::Mnemonic op, TR::RegisterDependencyConditions *cond,
                         TR::Node *node, TR::Node *fenceNode, TR::Instruction *precedingInstruction,
                         TR::CodeGenerator *cg)
      : TR::Instruction(op, node, cond, precedingInstruction, cg), _fenceNode(fenceNode)
      {
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsAdmin; }

   /**
    * @brief Gets fence node
    * @return fence node
    */
   TR::Node *getFenceNode() { return _fenceNode; }

   /**
    * @brief Assigns registers
    * @param[in] kindToBeAssigned : register kind
    */
   virtual void assignRegisters(TR_RegisterKinds kindToBeAssigned);

   /**
    * @brief Expand this instruction prior to binary encoding.
    *
    * @returns the last instruction in the expansion or this instruction if no expansion was
    *          performed.
    */
   virtual TR::Instruction *expandInstruction();

   /**
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();

   /**
    * @brief Estimates binary length
    * @param[in] currentEstimate : current estimated length
    * @return estimated binary length
    */
   virtual int32_t estimateBinaryLength(int32_t currentEstimate);
   };

class ARM64Trg1Instruction : public TR::Instruction
   {
   TR::Register *_target1Register;

   public:

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1Instruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *treg, TR::CodeGenerator *cg)
      : TR::Instruction(op, node, cg), _target1Register(treg)
      {
      useRegister(treg);
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1Instruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *treg,
                        TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : TR::Instruction(op, node, precedingInstruction, cg), _target1Register(treg)
      {
      useRegister(treg);
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] cond : register dependency conditions
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1Instruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *treg,
                        TR::RegisterDependencyConditions *cond, TR::CodeGenerator *cg)
      : TR::Instruction(op, node, cond, cg), _target1Register(treg)
      {
      useRegister(treg);
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] cond : register dependency conditions
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1Instruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *treg,
                        TR::RegisterDependencyConditions *cond, TR::Instruction *precedingInstruction,
                        TR::CodeGenerator *cg)
      : TR::Instruction(op, node, cond, precedingInstruction, cg), _target1Register(treg)
      {
      useRegister(treg);
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsTrg1; }

   /**
    * @brief Gets target register
    * @return target register
    */
   TR::Register *getTargetRegister() {return _target1Register;}
   /**
    * @brief Sets target register
    * @param[in] tr : target register
    * @return target register
    */
   TR::Register *setTargetRegister(TR::Register *tr) {return (_target1Register = tr);}

   /**
    * @brief Sets target register in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertTargetRegister(uint32_t *instruction)
      {
      TR::RealRegister *target = toRealRegister(_target1Register);
      target->setRegisterFieldRD(instruction);
      }

   /**
    * @brief Answers whether this instruction references the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction references the virtual register
    */
   virtual bool refsRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction uses the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction uses the virtual register
    */
   virtual bool usesRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction defines the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction defines the virtual register
    */
   virtual bool defsRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction defines the given real register
    * @param[in] reg : real register
    * @return true when the instruction defines the real register
    */
   virtual bool defsRealRegister(TR::Register *reg);
   /**
    * @brief Assigns registers
    * @param[in] kindToBeAssigned : register kind
    */
   virtual void assignRegisters(TR_RegisterKinds kindToBeAssigned);

   /**
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();
   };

class ARM64Trg1CondInstruction : public ARM64Trg1Instruction
   {
   TR::ARM64ConditionCode _cc;

   public:

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] cc : branch condition code
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1CondInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *treg,
                            TR::ARM64ConditionCode cc, TR::CodeGenerator *cg)
      : ARM64Trg1Instruction(op, node, treg, cg), _cc(cc)
      {
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] cc : branch condition code
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1CondInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *treg,
                            TR::ARM64ConditionCode cc, TR::Instruction *precedingInstruction,
                            TR::CodeGenerator *cg)
      : ARM64Trg1Instruction(op, node, treg, precedingInstruction, cg), _cc(cc)
      {
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsTrg1Cond; }

   /**
    * @brief Gets condition code
    * @return condition code
    */
   TR::ARM64ConditionCode getConditionCode() {return _cc;}
   /**
    * @brief Sets condition code
    * @param[in] cc : condition code
    * @return condition code
    */
   TR::ARM64ConditionCode setConditionCode(TR::ARM64ConditionCode cc) {return (_cc = cc);}

   /**
    * @brief Sets condition code field in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertConditionCodeField(uint32_t *instruction)
      {
      *instruction |= ((_cc & 0xf) << 12);
      }

   /**
    * @brief Sets zero register in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertZeroRegister(uint32_t *instruction)
      {
      TR::RealRegister *zeroReg = cg()->machine()->getRealRegister(TR::RealRegister::xzr);
      zeroReg->setRegisterFieldRM(instruction);
      zeroReg->setRegisterFieldRN(instruction);
      }

   /**
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();
   };

class ARM64Trg1ImmInstruction : public ARM64Trg1Instruction
   {
   uint32_t _sourceImmediate;

   public:

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] imm : immediate value
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1ImmInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *treg,
                            uint32_t imm, TR::CodeGenerator *cg)
      : ARM64Trg1Instruction(op, node, treg, cg), _sourceImmediate(imm)
      {
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] imm : immediate value
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1ImmInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *treg,
                            uint32_t imm,
                            TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : ARM64Trg1Instruction(op, node, treg, precedingInstruction, cg), _sourceImmediate(imm)
      {
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsTrg1Imm; }

   /**
    * @brief Gets source immediate
    * @return source immediate
    */
   uint32_t getSourceImmediate() {return _sourceImmediate;}
   /**
    * @brief Sets source immediate
    * @param[in] si : immediate value
    * @return source immediate
    */
   uint32_t setSourceImmediate(uint32_t si) {return (_sourceImmediate = si);}

   /**
    * @brief Sets immediate field in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertImmediateField(uint32_t *instruction)
      {
      TR::InstOpCode::Mnemonic op = getOpCodeValue();

      if (op == TR::InstOpCode::movzx || op == TR::InstOpCode::movzw ||
          op == TR::InstOpCode::movnx || op == TR::InstOpCode::movnw ||
          op == TR::InstOpCode::movkx || op == TR::InstOpCode::movkw)
         {
         *instruction |= ((_sourceImmediate & 0x3ffff) << 5);
         }
      else if (op == TR::InstOpCode::fmovimms || op == TR::InstOpCode::fmovimmd)
         {
         *instruction |= ((_sourceImmediate & 0xFF) << 13);
         }
      else if (op == TR::InstOpCode::adr || op == TR::InstOpCode::adrp)
         {
         *instruction |= ((_sourceImmediate & 0x1ffffc) << 3) | ((_sourceImmediate & 0x3) << 29);
         }
      else if (op == TR::InstOpCode::vmovi16b || op == TR::InstOpCode::vmovi8h ||
               op == TR::InstOpCode::vmovi4s || op == TR::InstOpCode::vmovi4s_one ||
               op == TR::InstOpCode::vmovi2s || op == TR::InstOpCode::movid ||
               op == TR::InstOpCode::vmovi2d || op == TR::InstOpCode::vfmov4s ||
               op == TR::InstOpCode::vfmov2d || op == TR::InstOpCode::vmvni8h ||
               op == TR::InstOpCode::vmvni4s || op == TR::InstOpCode::vmvni4s_one ||
               op == TR::InstOpCode::vbicimm8h || op == TR::InstOpCode::vbicimm4s ||
               op == TR::InstOpCode::vorrimm8h || op == TR::InstOpCode::vorrimm4s)
         {
         *instruction |= ((_sourceImmediate & 0xe0) << 11) | ((_sourceImmediate & 0x1f) << 5);
         }
      else
         {
         TR_ASSERT(false, "Unsupported opcode in ARM64Trg1ImmInstruction.");
         }
      }

   /**
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();
   };

class ARM64Trg1ImmShiftedInstruction : public ARM64Trg1ImmInstruction
   {
   uint32_t _shiftAmount;

   public:
   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] imm : immediate value
    * @param[in] shiftAmount : shiftAmount
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1ImmShiftedInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *treg,
                            uint32_t imm, uint32_t shiftAmount, TR::CodeGenerator *cg)
      : ARM64Trg1ImmInstruction(op, node, treg, imm, cg), _shiftAmount(shiftAmount)
      {
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] imm : immediate value
    * @param[in] shiftAmount : shiftAmount
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1ImmShiftedInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *treg,
                            uint32_t imm, uint32_t shiftAmount,
                            TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : ARM64Trg1ImmInstruction(op, node, treg, imm, precedingInstruction, cg), _shiftAmount(shiftAmount)
      {
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsTrg1ImmShifted; }

   /**
    * @brief Gets shift amount
    * @return shift amount
    */
   uint32_t getShiftAmount() {return _shiftAmount;}
   /**
    * @brief Sets shift amount
    * @param[in] sa : shift amount
    * @return shift amount
    */
   uint32_t setShiftAmount(uint32_t sa) {return (_shiftAmount = sa);}

   /**
    * @brief Sets shift type and amount in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertShift(uint32_t *instruction)
      {
      TR::InstOpCode::Mnemonic op = getOpCodeValue();
      if ((op == TR::InstOpCode::vmovi8h) || (op == TR::InstOpCode::vmvni8h) ||
          (op == TR::InstOpCode::vbicimm8h) || (op == TR::InstOpCode::vorrimm8h))
         {
         TR_ASSERT_FATAL((_shiftAmount == 0) || (_shiftAmount == 8), "shiftAmount other than 0 or 8 is not allowed for vmovi8h, vmvni8h, vbicimm8h and vorrimm8h");
         *instruction |= (_shiftAmount >> 3) << 13;
         }
      else if ((op == TR::InstOpCode::vmovi4s) || (op == TR::InstOpCode::vmvni4s) ||
               (op == TR::InstOpCode::vbicimm4s) || (op == TR::InstOpCode::vorrimm4s))
         {
         TR_ASSERT_FATAL((_shiftAmount == 0) || (_shiftAmount == 8) || (_shiftAmount == 16) || (_shiftAmount == 24),
                        "shiftAmount other than 0, 8, 16, or 24 is not allowed for vmovi4s, vmvni4s, vbicimm4s and vorrimm4s");
         *instruction |= (_shiftAmount >> 3) << 13;
         }
      else if ((op == TR::InstOpCode::vmovi4s_one) || (op == TR::InstOpCode::vmvni4s_one))
         {
         TR_ASSERT_FATAL((_shiftAmount == 8) || (_shiftAmount == 16), "shiftAmount other than 8 or 16 is not allowed for vmovi8h and vmvni8h");

         *instruction |= (_shiftAmount >> 4) << 12;
         }
      else
         {
         TR_ASSERT_FATAL(false, "Unsupported opcode in ARM64Trg1ImmShiftedInstruction.");
         }
      }

   /**
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();
   };

class ARM64Trg1ImmSymInstruction : public ARM64Trg1ImmInstruction
   {
   TR::Symbol *_symbol;

   public:
   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] imm : immediate value
    * @param[in] sym : symbol
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1ImmSymInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *treg,
                            uint32_t imm, TR::Symbol *sym, TR::CodeGenerator *cg)
      : ARM64Trg1ImmInstruction(op, node, treg, imm, cg), _symbol(sym)
      {
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] imm : immediate value
    * @param[in] sym : symbol
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1ImmSymInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *treg,
                            uint32_t imm, TR::Symbol *sym,
                            TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : ARM64Trg1ImmInstruction(op, node, treg, imm, precedingInstruction, cg), _symbol(sym)
      {
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsTrg1ImmSym; }

   /**
    *
    * @brief Gets symbol
    * @return symbol
    */
   TR::Symbol *getSymbol() {return _symbol;}

   /**
    * @brief Sets immediate field in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertImmediateField(uint32_t *instruction)
      {
      TR::InstOpCode::Mnemonic op = getOpCodeValue();
      if (op == TR::InstOpCode::adr || op == TR::InstOpCode::adrp)
         {
         *instruction |= ((getSourceImmediate() & 0x1ffffc) << 3) | ((getSourceImmediate() & 0x3) << 29);
         }
      else
         {
         *instruction |= ((getSourceImmediate() & 0x7ffff) << 5);
         }
      }

   /**
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();
   };

class ARM64Trg1Src1Instruction : public ARM64Trg1Instruction
   {
   TR::Register *_source1Register;

   public:

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] sreg : source register
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1Src1Instruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *treg,
                             TR::Register *sreg, TR::CodeGenerator *cg)
      : ARM64Trg1Instruction(op, node, treg, cg), _source1Register(sreg)
      {
      useRegister(sreg);
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] sreg : source register
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1Src1Instruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *treg,
                             TR::Register *sreg,
                             TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : ARM64Trg1Instruction(op, node, treg, precedingInstruction, cg), _source1Register(sreg)
      {
      useRegister(sreg);
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] sreg : source register
    * @param[in] cond : register dependency conditions
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1Src1Instruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *treg,
                             TR::Register *sreg, TR::RegisterDependencyConditions *cond, TR::CodeGenerator *cg)
      : ARM64Trg1Instruction(op, node, treg, cond, cg), _source1Register(sreg)
      {
      useRegister(sreg);
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] sreg : source register
    * @param[in] cond : register dependency conditions
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1Src1Instruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *treg,
                             TR::Register *sreg, TR::RegisterDependencyConditions *cond,
                             TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : ARM64Trg1Instruction(op, node, treg, cond, precedingInstruction, cg), _source1Register(sreg)
      {
      useRegister(sreg);
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsTrg1Src1; }

   /**
    * @brief Gets source register
    * @return source register
    */
   TR::Register *getSource1Register() {return _source1Register;}
   /**
    * @brief Sets source register
    * @param[in] sr : source register
    * @return source register
    */
   TR::Register *setSource1Register(TR::Register *sr) {return (_source1Register = sr);}

   /**
    * @brief Gets i-th source register
    * @param[in] i : index
    * @return i-th source register or NULL
    */
   virtual TR::Register *getSourceRegister(uint32_t i) {if (i==0) return _source1Register; return NULL;}

   /**
    * @brief Sets source register in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertSource1Register(uint32_t *instruction)
      {
      TR::RealRegister *source1 = toRealRegister(_source1Register);
      source1->setRegisterFieldRN(instruction);
      }

   /**
    * @brief Answers whether this instruction references the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction references the virtual register
    */
   virtual bool refsRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction uses the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction uses the virtual register
    */
   virtual bool usesRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction defines the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction defines the virtual register
    */
   virtual bool defsRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction defines the given real register
    * @param[in] reg : real register
    * @return true when the instruction defines the real register
    */
   virtual bool defsRealRegister(TR::Register *reg);
   /**
    * @brief Assigns registers
    * @param[in] kindToBeAssigned : register kind
    */
   virtual void assignRegisters(TR_RegisterKinds kindToBeAssigned);

   /**
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();
   };

/*
 * This class is designated to be used for alias instruction such as movw, movx, negw, negx
 */
class ARM64Trg1ZeroSrc1Instruction : public ARM64Trg1Src1Instruction
   {
   public:

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] sreg : source register
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1ZeroSrc1Instruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *treg,
                             TR::Register *sreg, TR::CodeGenerator *cg)
      : ARM64Trg1Src1Instruction(op, node, treg, sreg, cg)
      {
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] sreg : source register
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1ZeroSrc1Instruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *treg,
                             TR::Register *sreg,
                             TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : ARM64Trg1Src1Instruction(op, node, treg, sreg, precedingInstruction, cg)
      {
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsTrg1ZeroSrc1; }

   /**
    * @brief Sets source register in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertSource1Register(uint32_t *instruction)
      {
      TR::RealRegister *source1 = toRealRegister(getSource1Register());
      source1->setRegisterFieldRM(instruction);
      }

   /**
    * @brief Sets zero register in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertZeroRegister(uint32_t *instruction)
      {
      TR::RealRegister *zeroReg = cg()->machine()->getRealRegister(TR::RealRegister::xzr);
      zeroReg->setRegisterFieldRN(instruction);
      }

   /**
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();
   };

/*
 * This class is designated to be used for alias instruction such as movw, movx (bitmask immediate)
 */
class ARM64Trg1ZeroImmInstruction : public ARM64Trg1Instruction
   {
   uint32_t _source1Immediate;
   bool _Nbit;

   public:

   /*
    * @brief Constructor
    * @param[in]   op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in]  imm : immediate value
    * @param[in]   cg : CodeGenerator
    */
   ARM64Trg1ZeroImmInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *treg,
                             uint32_t imm, TR::CodeGenerator *cg)
      : ARM64Trg1Instruction(op, node, treg, cg), _source1Immediate(imm), _Nbit(false)
      {
      }

   /*
    * @brief Constructor
    * @param[in]   op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in]    N : N bit value
    * @param[in]  imm : immediate value
    * @param[in]   cg : CodeGenerator
    */
   ARM64Trg1ZeroImmInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *treg,
                             bool N, uint32_t imm, TR::CodeGenerator *cg)
      : ARM64Trg1Instruction(op, node, treg, cg), _source1Immediate(imm), _Nbit(N)
      {
      }

   /*
    * @brief Constructor
    * @param[in]                   op : instruction opcode
    * @param[in]                 node : node
    * @param[in]                 treg : target register
    * @param[in]                  imm : immediate value
    * @param[in] precedingInstruction : preceding instruction
    * @param[in]                   cg : CodeGenerator
    */
   ARM64Trg1ZeroImmInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *treg,
                             uint32_t imm,
                             TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : ARM64Trg1Instruction(op, node, treg, precedingInstruction, cg), _source1Immediate(imm), _Nbit(false)
      {
      }

   /*
    * @brief Constructor
    * @param[in]                   op : instruction opcode
    * @param[in]                 node : node
    * @param[in]                 treg : target register
    * @param[in]                    N : N bit value
    * @param[in]                  imm : immediate value
    * @param[in] precedingInstruction : preceding instruction
    * @param[in]                   cg : CodeGenerator
    */
   ARM64Trg1ZeroImmInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *treg,
                             bool N, uint32_t imm,
                             TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : ARM64Trg1Instruction(op, node, treg, precedingInstruction, cg), _source1Immediate(imm), _Nbit(N)
      {
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsTrg1ZeroImm; }

   /**
    * @brief Gets source immediate
    * @return source immediate
    */
   uint32_t getSourceImmediate() {return _source1Immediate;}
   /**
    * @brief Sets source immediate
    * @param[in] si : immediate value
    * @return source immediate
    */
   uint32_t setSourceImmediate(uint32_t si) {return (_source1Immediate = si);}

   /**
    * @brief Gets the N bit (bit 22)
    * @return N bit value
    */
   bool getNbit() { return _Nbit;}
   /**
    * @brief Sets the N bit (bit 22)
    * @param[in] n : N bit value
    * @return N bit value
    */
   bool setNbit(bool n) { return (_Nbit = n);}

   /**
    * @brief Sets zero register in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertZeroRegister(uint32_t *instruction)
      {
      TR::RealRegister *zeroReg = cg()->machine()->getRealRegister(TR::RealRegister::xzr);
      zeroReg->setRegisterFieldRN(instruction);
      }

   /**
    * @brief Sets immediate field in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertImmediateField(uint32_t *instruction)
      {
      *instruction |= ((_source1Immediate & 0xfff) << 10); /* imm12 */
      }

   /**
    * @brief Sets N bit (bit 22) field in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertNbit(uint32_t *instruction)
      {
      if (_Nbit)
         {
         *instruction |= (1 << 22);
         }
      }

   /**
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();
   };


class ARM64Trg1Src1ImmInstruction : public ARM64Trg1Src1Instruction
   {
   uint32_t _source1Immediate;
   bool _Nbit;

   public:

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] sreg : source register
    * @param[in] imm : immediate value
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1Src1ImmInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *treg,
                                TR::Register *sreg, uint32_t imm, TR::CodeGenerator *cg)
      : ARM64Trg1Src1Instruction(op, node, treg, sreg, cg), _source1Immediate(imm), _Nbit(false)
      {
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] sreg : source register
    * @param[in] N   : N bit value
    * @param[in] imm : immediate value
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1Src1ImmInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *treg,
                                TR::Register *sreg, bool N, uint32_t imm, TR::CodeGenerator *cg)
      : ARM64Trg1Src1Instruction(op, node, treg, sreg, cg), _source1Immediate(imm), _Nbit(N)
      {
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] sreg : source register
    * @param[in] imm : immediate value
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1Src1ImmInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *treg,
                                TR::Register *sreg, uint32_t imm,
                                TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : ARM64Trg1Src1Instruction(op, node, treg, sreg, precedingInstruction, cg), _source1Immediate(imm), _Nbit(false)
      {
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] sreg : source register
    * @param[in] N   : N bit value
    * @param[in] imm : immediate value
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1Src1ImmInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *treg,
                                TR::Register *sreg, bool N, uint32_t imm,
                                TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : ARM64Trg1Src1Instruction(op, node, treg, sreg, precedingInstruction, cg), _source1Immediate(imm), _Nbit(N)
      {
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] sreg : source register
    * @param[in] imm : immediate value
    * @param[in] cond : register dependency conditions
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1Src1ImmInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *treg,
                               TR::Register *sreg, uint32_t imm,
                               TR::RegisterDependencyConditions *cond, TR::CodeGenerator *cg)
      : ARM64Trg1Src1Instruction(op, node, treg, sreg, cond, cg), _source1Immediate(imm), _Nbit(false)
      {
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] sreg : source register
    * @param[in] imm : immediate value
    * @param[in] cond : register dependency conditions
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1Src1ImmInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *treg,
                               TR::Register *sreg, uint32_t imm, TR::RegisterDependencyConditions *cond,
                               TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : ARM64Trg1Src1Instruction(op, node, treg, sreg, cond, precedingInstruction, cg), _source1Immediate(imm), _Nbit(false)
      {
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsTrg1Src1Imm; }

   /**
    * @brief Gets source immediate
    * @return source immediate
    */
   uint32_t getSourceImmediate() {return _source1Immediate;}
   /**
    * @brief Sets source immediate
    * @param[in] si : immediate value
    * @return source immediate
    */
   uint32_t setSourceImmediate(uint32_t si) {return (_source1Immediate = si);}

   /**
    * @brief Gets the N bit (bit 22)
    * @return N bit value
    */
   bool getNbit() { return _Nbit;}
   /**
    * @brief Sets the N bit (bit 22)
    * @param[in] n : N bit value
    * @return N bit value
    */ 
   bool setNbit(bool n) { return (_Nbit = n);}

   /**
    * @brief Sets immediate field in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertImmediateField(uint32_t *instruction)
      {
      TR::InstOpCode::Mnemonic op = getOpCodeValue();

      if ((op >= TR::InstOpCode::vshl16b) && (op <= TR::InstOpCode::vsri2d))
         {
         *instruction |= ((_source1Immediate & 0x7f) << 16); /* immh:immb */
         }
      else if ((op >= TR::InstOpCode::vdupe16b) && (op <= TR::InstOpCode::vinsxd))
         {
         *instruction |= ((_source1Immediate & 0x1f) << 16); /* imm5 */
         }
      else if ((op >= TR::InstOpCode::vinseb) && (op <= TR::InstOpCode::vinsed))
         {
         *instruction |= ((_source1Immediate & 0x3ff) << 11); /* imm5 and imm4 */
         }
      else
         {
         *instruction |= ((_source1Immediate & 0xfff) << 10); /* imm12 */
         }
      }

   /**
    * @brief Sets N bit (bit 22) field in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertNbit(uint32_t *instruction)
      {
      if (_Nbit)
         {
         *instruction |= (1 << 22);
         }
      }

   /**
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();
   };

class ARM64Trg1Src2Instruction : public ARM64Trg1Src1Instruction
   {
   TR::Register *_source2Register;

   public:

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] s1reg : source register 1
    * @param[in] s2reg : source register 2
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1Src2Instruction( TR::InstOpCode::Mnemonic op,
                             TR::Node *node,
                             TR::Register *treg,
                             TR::Register *s1reg,
                             TR::Register *s2reg, TR::CodeGenerator *cg)
      : ARM64Trg1Src1Instruction(op, node, treg, s1reg, cg), _source2Register(s2reg)
      {
      useRegister(s2reg);
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] s1reg : source register 1
    * @param[in] s2reg : source register 2
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1Src2Instruction( TR::InstOpCode::Mnemonic op,
                             TR::Node *node,
                             TR::Register *treg,
                             TR::Register *s1reg,
                             TR::Register *s2reg,
                             TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : ARM64Trg1Src1Instruction(op, node, treg, s1reg, precedingInstruction, cg),
                             _source2Register(s2reg)
      {
      useRegister(s2reg);
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] s1reg : source register 1
    * @param[in] s2reg : source register 2
    * @param[in] cond : register dependency conditions
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1Src2Instruction( TR::InstOpCode::Mnemonic op,
                             TR::Node *node,
                             TR::Register *treg,
                             TR::Register *s1reg,
                             TR::Register *s2reg,
                             TR::RegisterDependencyConditions *cond,
                             TR::CodeGenerator *cg)
      : ARM64Trg1Src1Instruction(op, node, treg, s1reg, cond, cg), _source2Register(s2reg)
      {
      useRegister(s2reg);
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] s1reg : source register 1
    * @param[in] s2reg : source register 2
    * @param[in] cond : register dependency conditions
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1Src2Instruction( TR::InstOpCode::Mnemonic op,
                             TR::Node *node,
                             TR::Register *treg,
                             TR::Register *s1reg,
                             TR::Register *s2reg,
                             TR::RegisterDependencyConditions *cond,
                             TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : ARM64Trg1Src1Instruction(op, node, treg, s1reg, cond, precedingInstruction, cg),
                             _source2Register(s2reg)
      {
      useRegister(s2reg);
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsTrg1Src2; }

   /**
    * @brief Gets source register 2
    * @return source register 2
    */
   TR::Register *getSource2Register() {return _source2Register;}
   /**
    * @brief Sets source register 2
    * @param[in] sr : source register 2
    * @return source register 2
    */
   TR::Register *setSource2Register(TR::Register *sr) {return (_source2Register = sr);}

   /**
    * @brief Gets i-th source register
    * @param[in] i : index
    * @return i-th source register or NULL
    */
   virtual TR::Register *getSourceRegister(uint32_t i) {if      (i==0) return getSource1Register();
                                               else if (i==1) return _source2Register; return NULL;}

   /**
    * @brief Sets source register 2 in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertSource2Register(uint32_t *instruction)
      {
      TR::RealRegister *source2 = toRealRegister(_source2Register);
      source2->setRegisterFieldRM(instruction);
      }

   /**
    * @brief Answers whether this instruction references the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction references the virtual register
    */
   virtual bool refsRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction uses the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction uses the virtual register
    */
   virtual bool usesRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction defines the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction defines the virtual register
    */
   virtual bool defsRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction defines the given real register
    * @param[in] reg : real register
    * @return true when the instruction defines the real register
    */
   virtual bool defsRealRegister(TR::Register *reg);
   /**
    * @brief Assigns registers
    * @param[in] kindToBeAssigned : register kind
    */
   virtual void assignRegisters(TR_RegisterKinds kindToBeAssigned);

   /**
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();
   };


class ARM64CondTrg1Src2Instruction : public ARM64Trg1Src2Instruction
   {
   TR::ARM64ConditionCode _cc;

   public:

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] s1reg : source register 1
    * @param[in] s2reg : source register 2
    * @param[in] cc : branch condition code
    * @param[in] cg : CodeGenerator
    */
   ARM64CondTrg1Src2Instruction( TR::InstOpCode::Mnemonic op,
                             TR::Node *node,
                             TR::Register *treg,
                             TR::Register *s1reg,
                             TR::Register *s2reg,
                             TR::ARM64ConditionCode cc,
                             TR::CodeGenerator *cg)
      : ARM64Trg1Src2Instruction(op, node, treg, s1reg, s2reg, cg), _cc(cc)
      {
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] s1reg : source register 1
    * @param[in] s2reg : source register 2
    * @param[in] cc : branch condition code
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64CondTrg1Src2Instruction( TR::InstOpCode::Mnemonic op,
                             TR::Node *node,
                             TR::Register *treg,
                             TR::Register *s1reg,
                             TR::Register *s2reg,
                             TR::ARM64ConditionCode cc,
                             TR::Instruction *precedingInstruction,
                             TR::CodeGenerator *cg)
      : ARM64Trg1Src2Instruction(op, node, treg, s1reg, s2reg, precedingInstruction, cg), _cc(cc)
      {
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] s1reg : source register 1
    * @param[in] s2reg : source register 2
    * @param[in] cc : branch condition code
    * @param[in] cond : Register Dependency Condition
    * @param[in] cg : CodeGenerator
    */
   ARM64CondTrg1Src2Instruction( TR::InstOpCode::Mnemonic op,
                             TR::Node *node,
                             TR::Register *treg,
                             TR::Register *s1reg,
                             TR::Register *s2reg,
                             TR::ARM64ConditionCode cc,
                             TR::RegisterDependencyConditions *cond,
                             TR::CodeGenerator *cg)
      : ARM64Trg1Src2Instruction(op, node, treg, s1reg, s2reg, cond, cg), _cc(cc)
      {
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] s1reg : source register 1
    * @param[in] s2reg : source register 2
    * @param[in] cc : branch condition code
    * @param[in] cond : Register Dependency Condition
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64CondTrg1Src2Instruction( TR::InstOpCode::Mnemonic op,
                             TR::Node *node,
                             TR::Register *treg,
                             TR::Register *s1reg,
                             TR::Register *s2reg,
                             TR::ARM64ConditionCode cc,
                             TR::RegisterDependencyConditions *cond,
                             TR::Instruction *precedingInstruction,
                             TR::CodeGenerator *cg)
      : ARM64Trg1Src2Instruction(op, node, treg, s1reg, s2reg, cond, precedingInstruction, cg), _cc(cc)
      {
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsCondTrg1Src2; }

   /**
    * @brief Gets condition code
    * @return condition code
    */
   TR::ARM64ConditionCode getConditionCode() {return _cc;}

   /**
    * @brief Sets condition code
    * @param[in] cc : condition code
    * @return condition code
    */
   TR::ARM64ConditionCode setConditionCode(TR::ARM64ConditionCode cc)
      {
      return (_cc = cc);
      }

   /**
    * @brief Sets condition code in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertConditionCodeField(uint32_t *instruction)
      {
      *instruction |= ((_cc & 0xf) << 12);
      }

   /**
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();
};

class ARM64Trg1Src2ImmInstruction : public ARM64Trg1Src2Instruction
   {
   uint32_t _sourceImmediate; /* imm4 */

   public:

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] s1reg : source register 1
    * @param[in] s2reg : source register 2
    * @param[in] imm : immediate value
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1Src2ImmInstruction( TR::InstOpCode::Mnemonic op,
                             TR::Node *node,
                             TR::Register *treg,
                             TR::Register *s1reg,
                             TR::Register *s2reg,
                             uint32_t imm, TR::CodeGenerator *cg)
      : ARM64Trg1Src2Instruction(op, node, treg, s1reg, s2reg, cg), _sourceImmediate(imm)
      {
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] s1reg : source register 1
    * @param[in] s2reg : source register 2
    * @param[in] imm : immediate value
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1Src2ImmInstruction( TR::InstOpCode::Mnemonic op,
                             TR::Node *node,
                             TR::Register *treg,
                             TR::Register *s1reg,
                             TR::Register *s2reg,
                             uint32_t imm,
                             TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : ARM64Trg1Src2Instruction(op, node, treg, s1reg, s2reg, precedingInstruction, cg),
        _sourceImmediate(imm)
      {
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsTrg1Src2Imm; }

   /**
    * @brief Gets source immediate
    * @return source immediate
    */
   uint32_t getSourceImmediate() {return _sourceImmediate;}
   /**
    * @brief Sets source immediate
    * @param[in] si : immediate value
    * @return source immediate
    */
   uint32_t setSourceImmediate(uint32_t si) {return (_sourceImmediate = si);}

   /**
    * @brief Sets immediate field in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertImmediateField(uint32_t *instruction)
      {
      *instruction |= ((_sourceImmediate & 0xf) << 11);
      }

   /**
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();
   };

class ARM64Trg1Src2ShiftedInstruction : public ARM64Trg1Src2Instruction
   {
   ARM64ShiftCode _shiftType;
   uint32_t _shiftAmount; /* imm6 */

   public:

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] s1reg : source register 1
    * @param[in] s2reg : source register 2
    * @param[in] shiftType : shift type
    * @param[in] shiftAmount : shift amount
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1Src2ShiftedInstruction( TR::InstOpCode::Mnemonic op,
                             TR::Node *node,
                             TR::Register *treg,
                             TR::Register *s1reg,
                             TR::Register *s2reg,
                             ARM64ShiftCode shiftType,
                             uint32_t shiftAmount, TR::CodeGenerator *cg)
      : ARM64Trg1Src2Instruction(op, node, treg, s1reg, s2reg, cg), _shiftType(shiftType), _shiftAmount(shiftAmount)
      {
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] s1reg : source register 1
    * @param[in] s2reg : source register 2
    * @param[in] shiftType : shift type
    * @param[in] shiftAmount : shift amount
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1Src2ShiftedInstruction( TR::InstOpCode::Mnemonic op,
                             TR::Node *node,
                             TR::Register *treg,
                             TR::Register *s1reg,
                             TR::Register *s2reg,
                             ARM64ShiftCode shiftType,
                             uint32_t shiftAmount,
                             TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : ARM64Trg1Src2Instruction(op, node, treg, s1reg, s2reg, precedingInstruction, cg),
        _shiftType(shiftType), _shiftAmount(shiftAmount)
      {
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsTrg1Src2Shifted; }

   /**
    * @brief Gets shift type
    * @return shift type
    */
   ARM64ShiftCode getShiftType() {return _shiftType;}
   /**
    * @brief Sets shift type
    * @param[in] st : shift type
    * @return shift type
    */
   ARM64ShiftCode setShiftType(ARM64ShiftCode st) {return (_shiftType = st);}

   /**
    * @brief Gets shift amount
    * @return shift amount
    */
   uint32_t getShiftAmount() {return _shiftAmount;}
   /**
    * @brief Sets shift amount
    * @param[in] st : shift amount
    * @return shift amount
    */
   uint32_t setShiftAmount(uint32_t sa) {return (_shiftAmount = sa);}

   /**
    * @brief Sets shift type and amount in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertShift(uint32_t *instruction)
      {
      *instruction |= ((_shiftType & 0x3) << 22) | ((_shiftAmount & 0x3f) << 10);
      }

   /**
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();
   };

class ARM64Trg1Src2ExtendedInstruction : public ARM64Trg1Src2Instruction
   {
   ARM64ExtendCode _extendType;
   uint32_t _shiftAmount; /* imm3 */

   public:

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] s1reg : source register 1
    * @param[in] s2reg : source register 2
    * @param[in] extendType : extend type
    * @param[in] shiftAmount : shift amount
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1Src2ExtendedInstruction( TR::InstOpCode::Mnemonic op,
                             TR::Node *node,
                             TR::Register *treg,
                             TR::Register *s1reg,
                             TR::Register *s2reg,
                             ARM64ExtendCode extendType,
                             uint32_t shiftAmount, TR::CodeGenerator *cg)
      : ARM64Trg1Src2Instruction(op, node, treg, s1reg, s2reg, cg), _extendType(extendType), _shiftAmount(shiftAmount)
      {
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] s1reg : source register 1
    * @param[in] s2reg : source register 2
    * @param[in] extendType : extend type
    * @param[in] shiftAmount : shift amount
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1Src2ExtendedInstruction( TR::InstOpCode::Mnemonic op,
                             TR::Node *node,
                             TR::Register *treg,
                             TR::Register *s1reg,
                             TR::Register *s2reg,
                             ARM64ExtendCode extendType,
                             uint32_t shiftAmount,
                             TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : ARM64Trg1Src2Instruction(op, node, treg, s1reg, s2reg, precedingInstruction, cg),
        _extendType(extendType), _shiftAmount(shiftAmount)
      {
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsTrg1Src2Extended; }

   /**
    * @brief Gets extend type
    * @return extend type
    */
   ARM64ExtendCode getExtendType() {return _extendType;}
   /**
    * @brief Sets extend type
    * @param[in] st : extend type
    * @return extend type
    */
   ARM64ExtendCode setExtendType(ARM64ExtendCode et) {return (_extendType = et);}

   /**
    * @brief Gets shift amount
    * @return shift amount
    */
   uint32_t getShiftAmount() {return _shiftAmount;}
   /**
    * @brief Sets shift amount
    * @param[in] st : shift amount
    * @return shift amount
    */
   uint32_t setShiftAmount(uint32_t sa) {return (_shiftAmount = sa);}

   /**
    * @brief Sets extend type and amount in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertExtend(uint32_t *instruction)
      {
      *instruction |= ((_extendType & 0x7) << 13) | ((_shiftAmount & 0x7) << 10);
      }

   /**
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();
   };

class ARM64Trg1Src2IndexedElementInstruction : public ARM64Trg1Src2Instruction
   {
   uint32_t _index;

   public:

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] s1reg : source register 1
    * @param[in] s2reg : source register 2
    * @param[in] index : index of element in s2reg
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1Src2IndexedElementInstruction(TR::InstOpCode::Mnemonic op,
                             TR::Node *node,
                             TR::Register *treg,
                             TR::Register *s1reg,
                             TR::Register *s2reg,
                             uint32_t index, TR::CodeGenerator *cg)
      : ARM64Trg1Src2Instruction(op, node, treg, s1reg, s2reg, cg), _index(index)
      {
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] s1reg : source register 1
    * @param[in] s2reg : source register 2
    * @param[in] index : index of element in s2reg
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1Src2IndexedElementInstruction(TR::InstOpCode::Mnemonic op,
                             TR::Node *node,
                             TR::Register *treg,
                             TR::Register *s1reg,
                             TR::Register *s2reg,
                             uint32_t index,
                             TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : ARM64Trg1Src2Instruction(op, node, treg, s1reg, s2reg, precedingInstruction, cg),
        _index(index)
      {
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsTrg1Src2IndexedElement; }

   /**
    * @brief Gets index
    * @return index of element in s2reg
    */
   uint32_t getIndex() {return _index;}

   /**
    * @brief Sets index in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertIndex(uint32_t *instruction);

   /**
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();
   };

/*
 * This class is designated to be used for alias instruction such as mulw, mulx
 */
class ARM64Trg1Src2ZeroInstruction : public ARM64Trg1Src2Instruction
   {
   public:

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] s1reg : source register 1
    * @param[in] s2reg : source register 2
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1Src2ZeroInstruction( TR::InstOpCode::Mnemonic op,
                                 TR::Node *node,
                                 TR::Register *treg,
                                 TR::Register *s1reg,
                                 TR::Register *s2reg,
                                 TR::CodeGenerator *cg)
      : ARM64Trg1Src2Instruction(op, node, treg, s1reg, s2reg, cg)
      {
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] s1reg : source register 1
    * @param[in] s2reg : source register 2
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1Src2ZeroInstruction( TR::InstOpCode::Mnemonic op,
                                 TR::Node *node,
                                 TR::Register *treg,
                                 TR::Register *s1reg,
                                 TR::Register *s2reg,
                                 TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : ARM64Trg1Src2Instruction(op, node, treg, s1reg, s2reg, precedingInstruction, cg)
      {
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsTrg1Src2Zero; }

   /**
    * @brief Sets zero register in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertZeroRegister(uint32_t *instruction)
      {
      TR::RealRegister *zeroReg = cg()->machine()->getRealRegister(TR::RealRegister::xzr);
      zeroReg->setRegisterFieldRA(instruction);
      }

   /**
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();
   };

class ARM64Trg1Src3Instruction : public ARM64Trg1Src2Instruction
   {
   TR::Register *_source3Register;

   public:

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] s1reg : source register 1
    * @param[in] s2reg : source register 2
    * @param[in] s3reg : source register 3
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1Src3Instruction( TR::InstOpCode::Mnemonic op,
                             TR::Node *node,
                             TR::Register *treg,
                             TR::Register *s1reg,
                             TR::Register *s2reg,
                             TR::Register *s3reg, TR::CodeGenerator *cg)
      : ARM64Trg1Src2Instruction(op, node, treg, s1reg, s2reg, cg), _source3Register(s3reg)
      {
      useRegister(s3reg);
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] s1reg : source register 1
    * @param[in] s2reg : source register 2
    * @param[in] s3reg : source register 3
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1Src3Instruction( TR::InstOpCode::Mnemonic op,
                             TR::Node *node,
                             TR::Register *treg,
                             TR::Register *s1reg,
                             TR::Register *s2reg,
                             TR::Register *s3reg,
                             TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : ARM64Trg1Src2Instruction(op, node, treg, s1reg, s2reg, precedingInstruction, cg),
                             _source3Register(s3reg)
      {
      useRegister(s3reg);
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] s1reg : source register 1
    * @param[in] s2reg : source register 2
    * @param[in] s3reg : source register 3
    * @param[in] cond : register dependency conditions
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1Src3Instruction( TR::InstOpCode::Mnemonic op,
                             TR::Node *node,
                             TR::Register *treg,
                             TR::Register *s1reg,
                             TR::Register *s2reg,
                             TR::Register *s3reg,
                             TR::RegisterDependencyConditions *cond,
                             TR::CodeGenerator *cg)
      : ARM64Trg1Src2Instruction(op, node, treg, s1reg, s2reg, cond, cg), _source3Register(s3reg)
      {
      useRegister(s3reg);
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] s1reg : source register 1
    * @param[in] s2reg : source register 2
    * @param[in] s3reg : source register 3
    * @param[in] cond : register dependency conditions
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1Src3Instruction( TR::InstOpCode::Mnemonic op,
                             TR::Node *node,
                             TR::Register *treg,
                             TR::Register *s1reg,
                             TR::Register *s2reg,
                             TR::Register *s3reg,
                             TR::RegisterDependencyConditions *cond,
                             TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : ARM64Trg1Src2Instruction(op, node, treg, s1reg, s2reg, cond, precedingInstruction, cg),
                             _source3Register(s3reg)
      {
      useRegister(s3reg);
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsTrg1Src3; }

   /**
    * @brief Gets source register 3
    * @return source register 3
    */
   TR::Register *getSource3Register() {return _source3Register;}
   /**
    * @brief Sets source register 3
    * @param[in] sr : source register 3
    * @return source register 3
    */
   TR::Register *setSource3Register(TR::Register *sr) {return (_source3Register = sr);}

   /**
    * @brief Gets i-th source register
    * @param[in] i : index
    * @return i-th source register or NULL
    */
   virtual TR::Register *getSourceRegister(uint32_t i) {if      (i==0) return getSource1Register();
                                               else if (i==1) return getSource2Register();
                                               else if (i==2) return _source3Register; return NULL;}

   /**
    * @brief Sets source register 3 in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertSource3Register(uint32_t *instruction)
      {
      TR::RealRegister *source3 = toRealRegister(_source3Register);
      source3->setRegisterFieldRA(instruction);
      }

   /**
    * @brief Answers whether this instruction references the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction references the virtual register
    */
   virtual bool refsRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction uses the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction uses the virtual register
    */
   virtual bool usesRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction defines the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction defines the virtual register
    */
   virtual bool defsRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction defines the given real register
    * @param[in] reg : real register
    * @return true when the instruction defines the real register
    */
   virtual bool defsRealRegister(TR::Register *reg);
   /**
    * @brief Assigns registers
    * @param[in] kindToBeAssigned : register kind
    */
   virtual void assignRegisters(TR_RegisterKinds kindToBeAssigned);

   /**
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();
   };

class ARM64Trg1MemInstruction : public ARM64Trg1Instruction
   {
   TR::MemoryReference *_memoryReference;

   public:

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] mr : memory reference
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1MemInstruction(TR::InstOpCode::Mnemonic op,
                            TR::Node *node,
                            TR::Register *treg,
                            TR::MemoryReference *mr, TR::CodeGenerator *cg);

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] mr : memory reference
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1MemInstruction(TR::InstOpCode::Mnemonic op,
                            TR::Node *node,
                            TR::Register *treg,
                            TR::MemoryReference *mr,
                            TR::Instruction *precedingInstruction, TR::CodeGenerator *cg);

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsTrg1Mem; }

   /**
    * @brief Gets memory reference
    * @return memory reference
    */
   TR::MemoryReference *getMemoryReference() {return _memoryReference;}
   /**
    * @brief Sets memory reference
    * @param[in] mr : memory reference
    * @return memory reference
    */
   TR::MemoryReference *setMemoryReference(TR::MemoryReference *mr)
      {
      return (_memoryReference = mr);
      }

   virtual TR::Snippet *getSnippetForGC() {return getMemoryReference()->getUnresolvedSnippet();}

   /**
    * @brief Gets base register of memory reference
    * @return base register
    */
   virtual TR::Register *getMemoryBase() {return getMemoryReference()->getBaseRegister();}
   /**
    * @brief Gets index register of memory reference
    * @return index register
    */
   virtual TR::Register *getMemoryIndex() {return getMemoryReference()->getIndexRegister();}
   /**
    * @brief Gets offset of memory reference
    * @return offset
    */
   virtual int32_t getOffset() {return getMemoryReference()->getOffset();}

   /**
    * @brief Answers whether this instruction references the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction references the virtual register
    */
   virtual bool refsRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction uses the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction uses the virtual register
    */
   virtual bool usesRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction defines the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction defines the virtual register
    */
   virtual bool defsRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction defines the given real register
    * @param[in] reg : real register
    * @return true when the instruction defines the real register
    */
   virtual bool defsRealRegister(TR::Register *reg);
   /**
    * @brief Assigns registers
    * @param[in] kindToBeAssigned : register kind
    */
   virtual void assignRegisters(TR_RegisterKinds kindToBeAssigned);

   /**
    * @brief Expand this instruction prior to binary encoding.
    *
    * @returns the last instruction in the expansion or this instruction if no expansion was
    *          performed.
    */
   virtual TR::Instruction *expandInstruction();

   /**
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();

   /**
    * @brief Estimates binary length
    * @param[in] currentEstimate : current estimated length
    * @return estimated binary length
    */
   virtual int32_t estimateBinaryLength(int32_t currentEstimate);
   };

class ARM64Trg2MemInstruction : public ARM64Trg1MemInstruction
   {
   TR::Register *_target2Register;

   public:

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg1 : target register 1
    * @param[in] treg2 : target register 2
    * @param[in] mr : memory reference
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg2MemInstruction(TR::InstOpCode::Mnemonic op,
                            TR::Node *node,
                            TR::Register *treg1,
                            TR::Register *treg2,
                            TR::MemoryReference *mr, TR::CodeGenerator *cg)
      : ARM64Trg1MemInstruction(op, node, treg1, mr, cg), _target2Register(treg2)
      {
      useRegister(treg2);
      }
   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register 1
    * @param[in] treg2 : target register 2
    * @param[in] mr : memory reference
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg2MemInstruction(TR::InstOpCode::Mnemonic op,
                            TR::Node *node,
                            TR::Register *treg1,
                            TR::Register *treg2,
                            TR::MemoryReference *mr,
                            TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : ARM64Trg1MemInstruction(op, node, treg1, mr, precedingInstruction, cg), _target2Register(treg2)
      {
      useRegister(treg2);
      }
   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsTrg2Mem; }
   /**
    * @brief Gets target register 2
    * @return target register 2
    */
   TR::Register *getTarget2Register() {return _target2Register;}
   /**
    * @brief Sets target register 2
    * @param[in] tr : target register 2
    * @return target register 2
    */
   TR::Register *setTarget2Register(TR::Register *tr) {return (_target2Register = tr);}
   /**
    * @brief Sets target 2 register in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertTarget2Register(uint32_t *instruction)
      {
      TR::RealRegister *target2 = toRealRegister(_target2Register);
      target2->setRegisterFieldRT2(instruction);
      }
   /**
    * @brief Answers whether this instruction references the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction references the virtual register
    */
   virtual bool refsRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction uses the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction uses the virtual register
    */
   virtual bool usesRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction defines the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction defines the virtual register
    */
   virtual bool defsRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction defines the given real register
    * @param[in] reg : real register
    * @return true when the instruction defines the real register
    */
   virtual bool defsRealRegister(TR::Register *reg);
   /**
    * @brief Assigns registers
    * @param[in] kindToBeAssigned : register kind
    */
   virtual void assignRegisters(TR_RegisterKinds kindToBeAssigned);

   /**
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();

   /**
    * @brief Estimates binary length
    * @param[in] currentEstimate : current estimated length
    * @return estimated binary length
    */
   virtual int32_t estimateBinaryLength(int32_t currentEstimate);
   };

class ARM64MemInstruction : public TR::Instruction
   {
   TR::MemoryReference *_memoryReference;

   public:

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] mr : memory reference
    * @param[in] cg : CodeGenerator
    */
   ARM64MemInstruction(TR::InstOpCode::Mnemonic op,
                        TR::Node *node,
                        TR::MemoryReference *mr, TR::CodeGenerator *cg);

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] mr : memory reference
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64MemInstruction(TR::InstOpCode::Mnemonic op,
                        TR::Node *node,
                        TR::MemoryReference *mr,
                        TR::Instruction *precedingInstruction, TR::CodeGenerator *cg);

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsMem; }

   /**
    * @brief Gets memory reference
    * @return memory reference
    */
   TR::MemoryReference *getMemoryReference() {return _memoryReference;}
   /**
    * @brief Sets memory reference
    * @param[in] mr : memory reference
    * @return memory reference
    */
   TR::MemoryReference *setMemoryReference(TR::MemoryReference *mr)
      {
      return (_memoryReference = mr);
      }

   /**
    * @brief Gets base register of memory reference
    * @return base register
    */
   virtual TR::Register *getMemoryBase() {return getMemoryReference()->getBaseRegister();}
   /**
    * @brief Gets index register of memory reference
    * @return index register
    */
   virtual TR::Register *getMemoryIndex() {return getMemoryReference()->getIndexRegister();}
   /**
    * @brief Gets offset of memory reference
    * @return offset
    */
   virtual int32_t getOffset() {return getMemoryReference()->getOffset();}
   /**
    * @brief Answers whether this instruction references the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction references the virtual register
    */
   virtual bool refsRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction uses the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction uses the virtual register
    */
   virtual bool usesRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction defines the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction defines the virtual register
    */
   virtual bool defsRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction defines the given real register
    * @param[in] reg : real register
    * @return true when the instruction defines the real register
    */
   virtual bool defsRealRegister(TR::Register *reg);
   /**
    * @brief Assigns registers
    * @param[in] kindToBeAssigned : register kind
    */
   virtual void assignRegisters(TR_RegisterKinds kindToBeAssigned);

   /**
    * @brief Expand this instruction prior to binary encoding.
    *
    * @returns the last instruction in the expansion or this instruction if no expansion was
    *          performed.
    */
   virtual TR::Instruction *expandInstruction();

   /**
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();

   /**
    * @brief Estimates binary length
    * @param[in] currentEstimate : current estimated length
    * @return estimated binary length
    */
   virtual int32_t estimateBinaryLength(int32_t currentEstimate);
   };

class ARM64MemSrc1Instruction : public ARM64MemInstruction
   {
   TR::Register *_source1Register;

   public:

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] mr : memory reference
    * @param[in] sreg : source register
    * @param[in] cg : CodeGenerator
    */
   ARM64MemSrc1Instruction(TR::InstOpCode::Mnemonic op,
                            TR::Node *node,
                            TR::MemoryReference *mr,
                            TR::Register *sreg, TR::CodeGenerator *cg)
      : ARM64MemInstruction(op, node, mr, cg), _source1Register(sreg)
      {
      useRegister(sreg);
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] mr : memory reference
    * @param[in] sreg : source register
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64MemSrc1Instruction(TR::InstOpCode::Mnemonic op,
                            TR::Node *node,
                            TR::MemoryReference *mr,
                            TR::Register *sreg,
                            TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : ARM64MemInstruction(op, node, mr, precedingInstruction, cg), _source1Register(sreg)
      {
      useRegister(sreg);
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsMemSrc1; }

   /**
    * @brief Gets source register
    * @return source register
    */
   TR::Register *getSource1Register() {return _source1Register;}
   /**
    * @brief Sets source register
    * @param[in] sr : source register
    * @return source register
    */
   TR::Register *setSource1Register(TR::Register *sr) {return (_source1Register = sr);}

   virtual TR::Snippet *getSnippetForGC() {return getMemoryReference()->getUnresolvedSnippet();}

   /**
    * @brief Sets source register in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertSource1Register(uint32_t *instruction)
      {
      TR::RealRegister *source1 = toRealRegister(_source1Register);
      source1->setRegisterFieldRT(instruction);
      }

   /**
    * @brief Answers whether this instruction references the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction references the virtual register
    */
   virtual bool refsRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction uses the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction uses the virtual register
    */
   virtual bool usesRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction defines the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction defines the virtual register
    */
   virtual bool defsRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction defines the given real register
    * @param[in] reg : real register
    * @return true when the instruction defines the real register
    */
   virtual bool defsRealRegister(TR::Register *reg);
   /**
    * @brief Assigns registers
    * @param[in] kindToBeAssigned : register kind
    */
   virtual void assignRegisters(TR_RegisterKinds kindToBeAssigned);

   /**
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();

   /**
    * @brief Estimates binary length
    * @param[in] currentEstimate : current estimated length
    * @return estimated binary length
    */
   virtual int32_t estimateBinaryLength(int32_t currentEstimate);
   };

class ARM64MemSrc2Instruction : public ARM64MemSrc1Instruction
   {
   TR::Register *_source2Register;

   public:

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] mr : memory reference
    * @param[in] s1reg : source register 1
    * @param[in] s2reg : source register 2
    * @param[in] cg : CodeGenerator
    */
   ARM64MemSrc2Instruction(TR::InstOpCode::Mnemonic op,
                            TR::Node *node,
                            TR::MemoryReference *mr,
                            TR::Register *s1reg,
                            TR::Register *s2reg,
                            TR::CodeGenerator *cg)
      : ARM64MemSrc1Instruction(op, node, mr, s1reg, cg), _source2Register(s2reg)
      {
      useRegister(s2reg);
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] mr : memory reference
    * @param[in] s1reg : source register 1
    * @param[in] s2reg : source register 2
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64MemSrc2Instruction(TR::InstOpCode::Mnemonic op,
                            TR::Node *node,
                            TR::MemoryReference *mr,
                            TR::Register *s1reg,
                            TR::Register *s2reg,
                            TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : ARM64MemSrc1Instruction(op, node, mr, s1reg, precedingInstruction, cg), _source2Register(s2reg)
      {
      useRegister(s2reg);
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsMemSrc2; }

   /**
    * @brief Gets source register 2
    * @return source register 2
    */
   TR::Register *getSource2Register() {return _source2Register;}

   /**
    * @brief Sets source register 2
    * @param[in] sr : source register 2
    * @return source register 2
    */
   TR::Register *setSource2Register(TR::Register *sr) {return (_source2Register = sr);}

   virtual TR::Snippet *getSnippetForGC() {return getMemoryReference()->getUnresolvedSnippet();}

   /**
    * @brief Sets source register 2 in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertSource2Register(uint32_t *instruction)
      {
      TR::RealRegister *source2 = toRealRegister(_source2Register);
      source2->setRegisterFieldRT2(instruction);
      }

   /**
    * @brief Answers whether this instruction references the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction references the virtual register
    */
   virtual bool refsRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction uses the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction uses the virtual register
    */
   virtual bool usesRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction defines the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction defines the virtual register
    */
   virtual bool defsRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction defines the given real register
    * @param[in] reg : real register
    * @return true when the instruction defines the real register
    */
   virtual bool defsRealRegister(TR::Register *reg);
   /**
    * @brief Assigns registers
    * @param[in] kindToBeAssigned : register kind
    */
   virtual void assignRegisters(TR_RegisterKinds kindToBeAssigned);

   /**
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();

   /**
    * @brief Estimates binary length
    * @param[in] currentEstimate : current estimated length
    * @return estimated binary length
    */
   virtual int32_t estimateBinaryLength(int32_t currentEstimate);
   };

// for "store exclusive" instructions such as stxrx/stxrw
class ARM64Trg1MemSrc1Instruction : public ARM64Trg1MemInstruction
   {
   TR::Register *_source1Register;

   public:

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] mr : memory reference
    * @param[in] sreg : source register
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1MemSrc1Instruction(TR::InstOpCode::Mnemonic op,
                               TR::Node *node,
                               TR::Register *treg,
                               TR::MemoryReference *mr,
                               TR::Register *sreg, TR::CodeGenerator *cg)
      : ARM64Trg1MemInstruction(op, node, treg, mr, cg), _source1Register(sreg)
      {
      useRegister(sreg);
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] treg : target register
    * @param[in] mr : memory reference
    * @param[in] sreg : source register
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64Trg1MemSrc1Instruction(TR::InstOpCode::Mnemonic op,
                               TR::Node *node,
                               TR::Register *treg,
                               TR::MemoryReference *mr,
                               TR::Register *sreg,
                               TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : ARM64Trg1MemInstruction(op, node, treg, mr, precedingInstruction, cg), _source1Register(sreg)
      {
      useRegister(sreg);
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsTrg1MemSrc1; }

   /**
    * @brief Gets source register
    * @return source register
    */
   TR::Register *getSource1Register() {return _source1Register;}
   /**
    * @brief Sets source register
    * @param[in] sr : source register
    * @return source register
    */
   TR::Register *setSource1Register(TR::Register *sr) {return (_source1Register = sr);}

   virtual TR::Snippet *getSnippetForGC() {return getMemoryReference()->getUnresolvedSnippet();}

   /**
    * @brief Sets target register in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertTargetRegister(uint32_t *instruction)
      {
      TR::RealRegister *target = toRealRegister(getTargetRegister());
      target->setRegisterFieldRS(instruction);
      }

   /**
    * @brief Sets source register in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertSource1Register(uint32_t *instruction)
      {
      TR::RealRegister *source1 = toRealRegister(_source1Register);
      source1->setRegisterFieldRT(instruction);
      }

   /**
    * @brief Answers whether this instruction references the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction references the virtual register
    */
   virtual bool refsRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction uses the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction uses the virtual register
    */
   virtual bool usesRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction defines the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction defines the virtual register
    */
   virtual bool defsRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction defines the given real register
    * @param[in] reg : real register
    * @return true when the instruction defines the real register
    */
   virtual bool defsRealRegister(TR::Register *reg);
   /**
    * @brief Assigns registers
    * @param[in] kindToBeAssigned : register kind
    */
   virtual void assignRegisters(TR_RegisterKinds kindToBeAssigned);

   /**
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();

   /**
    * @brief Estimates binary length
    * @param[in] currentEstimate : current estimated length
    * @return estimated binary length
    */
   virtual int32_t estimateBinaryLength(int32_t currentEstimate);
   };

class ARM64MemImmInstruction : public ARM64MemInstruction
   {
   uint32_t _immediate; // Imm5

   public:
   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] mr : memory reference
    * @param[in] immediate : 5bit immediate
    * @param[in] cg : CodeGenerator
    */
   ARM64MemImmInstruction(TR::InstOpCode::Mnemonic op,
                        TR::Node *node,
                        TR::MemoryReference *mr,
                        uint32_t immediate,
                        TR::CodeGenerator *cg)
      : ARM64MemInstruction(op, node, mr, cg),  _immediate(immediate)
      {
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] mr : memory reference
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] immediate : 5bit immediate
    * @param[in] cg : CodeGenerator
    */
   ARM64MemImmInstruction(TR::InstOpCode::Mnemonic op,
                        TR::Node *node,
                        TR::MemoryReference *mr,
                        uint32_t immediate,
                        TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : ARM64MemInstruction(op, node, mr, precedingInstruction, cg), _immediate(immediate)
      {
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsMemImm; }

   /**
    * @brief Gets immediate
    * @return immediate
    */
   uint32_t getImmediate() {return _immediate;}
   /**
    * @brief Sets immediate
    * @param[in] immediate : immediate value
    * @return immediate
    */
   uint32_t setImmediate(uint32_t immediate) {return (_immediate = immediate);}

   /**
    * @brief Sets immediate field in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertImmediateField(uint32_t *instruction)
      {
      *instruction |= (_immediate & 0x1f); /* imm5 */
      }

   /**
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();
   };

class ARM64Src1Instruction : public TR::Instruction
   {
   TR::Register *_source1Register;

   public:

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] sreg : source register
    * @param[in] cg : CodeGenerator
    */
   ARM64Src1Instruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *sreg, TR::CodeGenerator *cg)
      : TR::Instruction(op, node, cg), _source1Register(sreg)
      {
      useRegister(sreg);
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] sreg : source register
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64Src1Instruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *sreg,
                        TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : TR::Instruction(op, node, precedingInstruction, cg), _source1Register(sreg)
      {
      useRegister(sreg);
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsSrc1; }

   /**
    * @brief Gets source register
    * @return source register
    */
   TR::Register *getSource1Register() {return _source1Register;}
   /**
    * @brief Sets source register
    * @param[in] tr : source register
    * @return source register
    */
   TR::Register *setSource1Register(TR::Register *tr) {return (_source1Register = tr);}

   /**
    * @brief Gets i-th source register
    * @param[in] i : index
    * @return i-th source register or NULL
    */
   virtual TR::Register *getSourceRegister(uint32_t i) {if (i==0) return _source1Register; return NULL;}

   /**
    * @brief Sets source register in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertSource1Register(uint32_t *instruction)
      {
      TR::RealRegister *source = toRealRegister(_source1Register);
      source->setRegisterFieldRN(instruction);
      }

   /**
    * @brief Answers whether this instruction references the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction references the virtual register
    */
   virtual bool refsRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction uses the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction uses the virtual register
    */
   virtual bool usesRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction defines the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction defines the virtual register
    */
   virtual bool defsRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction defines the given real register
    * @param[in] reg : real register
    * @return true when the instruction defines the real register
    */
   virtual bool defsRealRegister(TR::Register *reg);
   /**
    * @brief Assigns registers
    * @param[in] kindToBeAssigned : register kind
    */
   virtual void assignRegisters(TR_RegisterKinds kindToBeAssigned);

   /**
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();
   };

/*
 * This class is designated to be used for alias instruction such as cmpimmw, cmpimmx, tstimmw, tstimmx
 */
class ARM64ZeroSrc1ImmInstruction : public ARM64Src1Instruction
   {
   uint32_t _source1Immediate;
   bool _Nbit;

   public:

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] sreg : source register
    * @param[in] imm : immediate value
    * @param[in] cg : CodeGenerator
    */
   ARM64ZeroSrc1ImmInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node,
                                TR::Register *sreg, uint32_t imm, TR::CodeGenerator *cg)
      : ARM64Src1Instruction(op, node, sreg, cg), _source1Immediate(imm), _Nbit(false)
      {
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] sreg : source register
    * @param[in] N   : N bit value
    * @param[in] imm : immediate value
    * @param[in] cg : CodeGenerator
    */
   ARM64ZeroSrc1ImmInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node,
                                TR::Register *sreg, bool N, uint32_t imm, TR::CodeGenerator *cg)
      : ARM64Src1Instruction(op, node, sreg, cg), _source1Immediate(imm), _Nbit(N)
      {
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] sreg : source register
    * @param[in] imm : immediate value
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64ZeroSrc1ImmInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node,
                                TR::Register *sreg, uint32_t imm,
                                TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : ARM64Src1Instruction(op, node, sreg, precedingInstruction, cg), _source1Immediate(imm), _Nbit(false)
      {
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] sreg : source register
    * @param[in] N   : N bit value
    * @param[in] imm : immediate value
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64ZeroSrc1ImmInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node,
                                TR::Register *sreg, bool N, uint32_t imm,
                                TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : ARM64Src1Instruction(op, node, sreg, precedingInstruction, cg), _source1Immediate(imm), _Nbit(N)
      {
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsZeroSrc1Imm; }

   /**
    * @brief Gets source immediate
    * @return source immediate
    */
   uint32_t getSourceImmediate() {return _source1Immediate;}
   /**
    * @brief Sets source immediate
    * @param[in] si : immediate value
    * @return source immediate
    */
   uint32_t setSourceImmediate(uint32_t si) {return (_source1Immediate = si);}

   /**
    * @brief Gets the N bit (bit 22)
    * @return N bit value
    */
   bool getNbit() { return _Nbit;}
   /**
    * @brief Sets the N bit (bit 22)
    * @param[in] n : N bit value
    * @return N bit value
    */
   bool setNbit(bool n) { return (_Nbit = n);}

   /**
    * @brief Sets zero register in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertZeroRegister(uint32_t *instruction)
      {
      TR::RealRegister *zeroReg = cg()->machine()->getRealRegister(TR::RealRegister::xzr);
      zeroReg->setRegisterFieldRD(instruction);
      }

   /**
    * @brief Sets immediate field in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertImmediateField(uint32_t *instruction)
      {
      *instruction |= ((_source1Immediate & 0xfff) << 10); /* imm12 */
      }

   /**
    * @brief Sets N bit (bit 22) field in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertNbit(uint32_t *instruction)
      {
      if (_Nbit)
         {
         *instruction |= (1 << 22);
         }
      }

   /**
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();
   };

class ARM64Src2Instruction : public ARM64Src1Instruction
   {
   TR::Register *_source2Register;

   public:

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] s1reg : source register 1
    * @param[in] s2reg : source register 2
    * @param[in] cg : CodeGenerator
    */
   ARM64Src2Instruction( TR::InstOpCode::Mnemonic op,
                         TR::Node *node,
                         TR::Register *s1reg,
                         TR::Register *s2reg, TR::CodeGenerator *cg)
      : ARM64Src1Instruction(op, node, s1reg, cg), _source2Register(s2reg)
      {
      useRegister(s2reg);
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] s1reg : source register 1
    * @param[in] s2reg : source register 2
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64Src2Instruction( TR::InstOpCode::Mnemonic op,
                         TR::Node *node,
                         TR::Register *s1reg,
                         TR::Register *s2reg,
                         TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : ARM64Src1Instruction(op, node, s1reg, precedingInstruction, cg),
        _source2Register(s2reg)
      {
      useRegister(s2reg);
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsSrc2; }

   /**
    * @brief Gets source register 2
    * @return source register 2
    */
   TR::Register *getSource2Register() {return _source2Register;}
   /**
    * @brief Sets source register 2
    * @param[in] sr : source register 2
    * @return source register 2
    */
   TR::Register *setSource2Register(TR::Register *sr) {return (_source2Register = sr);}

   /**
    * @brief Gets i-th source register
    * @param[in] i : index
    * @return i-th source register or NULL
    */
   virtual TR::Register *getSourceRegister(uint32_t i) {if      (i==0) return getSource1Register();
                                                        else if (i==1) return _source2Register; return NULL;}

   /**
    * @brief Sets source register 2 in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertSource2Register(uint32_t *instruction)
      {
      TR::RealRegister *source2 = toRealRegister(_source2Register);
      source2->setRegisterFieldRM(instruction);
      }

   /**
    * @brief Answers whether this instruction references the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction references the virtual register
    */
   virtual bool refsRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction uses the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction uses the virtual register
    */
   virtual bool usesRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction defines the given virtual register
    * @param[in] reg : virtual register
    * @return true when the instruction defines the virtual register
    */
   virtual bool defsRegister(TR::Register *reg);
   /**
    * @brief Answers whether this instruction defines the given real register
    * @param[in] reg : real register
    * @return true when the instruction defines the real register
    */
   virtual bool defsRealRegister(TR::Register *reg);
   /**
    * @brief Assigns registers
    * @param[in] kindToBeAssigned : register kind
    */
   virtual void assignRegisters(TR_RegisterKinds kindToBeAssigned);

   /**
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();
   };

/*
 * This class is designated to be used for alias instruction such as cmpw, cmpx, tstw, tstx
 */
class ARM64ZeroSrc2Instruction : public ARM64Src2Instruction
   {
   public:

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] s1reg : source register 1
    * @param[in] s2reg : source register 2
    * @param[in] cg : CodeGenerator
    */
   ARM64ZeroSrc2Instruction( TR::InstOpCode::Mnemonic op,
                         TR::Node *node,
                         TR::Register *s1reg,
                         TR::Register *s2reg, TR::CodeGenerator *cg)
      : ARM64Src2Instruction(op, node, s1reg, s2reg, cg)
      {
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] s1reg : source register 1
    * @param[in] s2reg : source register 2
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64ZeroSrc2Instruction( TR::InstOpCode::Mnemonic op,
                         TR::Node *node,
                         TR::Register *s1reg,
                         TR::Register *s2reg,
                         TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : ARM64Src2Instruction(op, node, s1reg, s2reg, precedingInstruction, cg)
      {
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsZeroSrc2; }

   /**
    * @brief Sets zero register in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertZeroRegister(uint32_t *instruction)
      {
      TR::RealRegister *zeroReg = cg()->machine()->getRealRegister(TR::RealRegister::xzr);
      zeroReg->setRegisterFieldRD(instruction);
      }

   /**
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();
   };


class ARM64ConditionalCompareOp
   {
   TR::ARM64ConditionCode _cc;
   uint32_t _conditionFlags;

   public:

   ARM64ConditionalCompareOp(TR::ARM64ConditionCode cc, uint32_t conditionFlags)
      : _cc(cc), _conditionFlags(conditionFlags)
      {
      }

   /**
    * @brief Gets condition code
    * @return condition code
    */
   TR::ARM64ConditionCode getConditionCode() {return _cc;}
   /**
    * @brief Sets condition code
    * @param[in] cc : condition code
    * @return condition code
    */
   TR::ARM64ConditionCode setConditionCode(TR::ARM64ConditionCode cc) {return (_cc = cc);}

   /**
    * @brief Get the value of the condition flags
    * @return the value of the condition flags
    */
   uint32_t getConditionFlags() { return _conditionFlags;}

   /**
    * @brief Sets the condition flags
    * @param[in] conditionFlags : condition flags
    * @return the value of the condition flags
    */
   uint32_t setConditionFlags(uint32_t conditionFlags) { return (_conditionFlags = conditionFlags);}

   /**
    * @brief Sets condition code field in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertConditionCodeField(uint32_t *instruction)
      {
      *instruction |= ((_cc & 0xf) << 12);
      }

   /**
    * @brief Sets condition flags in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertConditionFlags(uint32_t *instruction)
      {
      TR_ASSERT_FATAL(constantIsUnsignedImm4(getConditionFlags()), "Condition flags exceeds 4 bits.");
      *instruction |= (_conditionFlags & 0xf);
      }
   };

class ARM64Src1ImmCondInstruction : public ARM64Src1Instruction, public ARM64ConditionalCompareOp
   {
   uint32_t _sourceImmediate;

   public:

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] s1reg : source register 1
    * @param[in] imm : immediate value
    * @param[in] cc : branch condition code
    * @param[in] conditionFlags : 4-bit NZCV condition flags
    * @param[in] cg : CodeGenerator
    */
   ARM64Src1ImmCondInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *s1reg,
                            uint32_t imm, TR::ARM64ConditionCode cc, uint32_t conditionFlags, TR::CodeGenerator *cg)
      : ARM64Src1Instruction(op, node, s1reg, cg), ARM64ConditionalCompareOp(cc, conditionFlags), _sourceImmediate(imm)
      {
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] s1reg : source register 1
    * @param[in] imm : immediate value
    * @param[in] cc : branch condition code
    * @param[in] conditionFlags : 4-bit NZCV condition flags
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64Src1ImmCondInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *s1reg,
                            uint32_t imm, TR::ARM64ConditionCode cc, uint32_t conditionFlags,
                            TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : ARM64Src1Instruction(op, node, s1reg, precedingInstruction, cg), ARM64ConditionalCompareOp(cc, conditionFlags), _sourceImmediate(imm)
      {
      }
   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsSrc1ImmCond; }
   /**
    * @brief Gets source immediate
    * @return source immediate
    */
   uint32_t getSourceImmediate() {return _sourceImmediate;}
   /**
    * @brief Sets source immediate
    * @param[in] si : immediate value
    * @return source immediate
    */
   uint32_t setSourceImmediate(uint32_t si) {return (_sourceImmediate = si);}

   /**
    * @brief Sets immediate field in binary encoding
    * @param[in] instruction : instruction cursor
    */
   void insertImmediateField(uint32_t *instruction)
      {
      TR_ASSERT_FATAL(constantIsUnsignedImm5(getSourceImmediate()), "Immediate value exceeds 5 bits.");
      *instruction |= ((getSourceImmediate() & 0x1f) << 16);
      }
   /**
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();
   };

class ARM64Src2CondInstruction : public ARM64Src2Instruction, public ARM64ConditionalCompareOp
   {

   public:

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] s1reg : source register 1
    * @param[in] s2reg : source register 2
    * @param[in] cc : branch condition code
    * @param[in] conditionFlags : 4-bit NZCV condition flags
    * @param[in] cg : CodeGenerator
    */
   ARM64Src2CondInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *s1reg,
                            TR::Register *s2reg, TR::ARM64ConditionCode cc, uint32_t conditionFlags, TR::CodeGenerator *cg)
      : ARM64Src2Instruction(op, node, s1reg, s2reg, cg), ARM64ConditionalCompareOp(cc, conditionFlags)
      {
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] s1reg : source register 1
    * @param[in] s2reg : source register 2
    * @param[in] cc : branch condition code
    * @param[in] conditionFlags : 4-bit NZCV condition flags
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64Src2CondInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Register *s1reg,
                           TR::Register *s2reg, TR::ARM64ConditionCode cc, uint32_t conditionFlags,
                            TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : ARM64Src2Instruction(op, node, s1reg, s2reg, precedingInstruction, cg), ARM64ConditionalCompareOp(cc, conditionFlags)
      {
      }
   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsSrc2Cond; }

   /**
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();
   };

class ARM64SynchronizationInstruction : public ARM64ImmInstruction
   {

   public:

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] imm : immediate value
    * @param[in] cg : CodeGenerator
    */
   ARM64SynchronizationInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node,
                                    uint32_t imm, TR::CodeGenerator *cg)
      : ARM64ImmInstruction(op, node, imm, cg)
      {
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] imm : immediate value
    * @param[in] preced : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64SynchronizationInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node,
                                    uint32_t imm, TR::Instruction *precedingInstruction,
                                    TR::CodeGenerator *cg)
      : ARM64ImmInstruction(op, node, imm, precedingInstruction, cg)
      {
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsSynchronization; }

   /**
    * @brief Encodes the immediate value into the instruction
    * @param[in] instruction : instruction address
    */
   virtual void insertImmediateField(uint32_t *instruction)
      {
      TR_ASSERT_FATAL(constantIsUnsignedImm4(getSourceImmediate()), "Immediate value exceeds 4 bits.");
      *instruction |= ((getSourceImmediate() & 0xF) << 8);
      }

   };

class ARM64ExceptionInstruction : public ARM64ImmInstruction
   {

   public:

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] imm : immediate value
    * @param[in] cg : CodeGenerator
    */
   ARM64ExceptionInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node,
                                    uint32_t imm, TR::CodeGenerator *cg)
      : ARM64ImmInstruction(op, node, imm, cg)
      {
      }

   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] imm : immediate value
    * @param[in] preced : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   ARM64ExceptionInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node,
                                    uint32_t imm, TR::Instruction *precedingInstruction,
                                    TR::CodeGenerator *cg)
      : ARM64ImmInstruction(op, node, imm, precedingInstruction, cg)
      {
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsException; }

   /**
    * @brief Encodes the immediate value into the instruction
    * @param[in] instruction : instruction address
    */
   virtual void insertImmediateField(uint32_t *instruction)
      {
      TR_ASSERT_FATAL(constantIsUnsignedImm16(getSourceImmediate()), "Immediate value exceeds 16 bits.");
      *instruction |= ((getSourceImmediate() & 0xFFFF) << 5);
      }

   };

#ifdef J9_PROJECT_SPECIFIC
class ARM64VirtualGuardNOPInstruction : public TR::ARM64LabelInstruction
   {
   private:
   TR_VirtualGuardSite *_site;

   public:
   ARM64VirtualGuardNOPInstruction(TR::Node                       *node,
                                 TR_VirtualGuardSite              *site,
                                 TR::RegisterDependencyConditions *cond,
                                 TR::LabelSymbol                  *sym,
                                 TR::CodeGenerator                *cg)
      : TR::ARM64LabelInstruction(TR::InstOpCode::vgnop, node, sym, cond, cg),
        _site(site)
      {
      }

   ARM64VirtualGuardNOPInstruction(TR::Node                       *node,
                                 TR_VirtualGuardSite              *site,
                                 TR::RegisterDependencyConditions *cond,
                                 TR::LabelSymbol                  *sym,
                                 TR::Instruction                  *precedingInstruction,
                                 TR::CodeGenerator                *cg)
      : TR::ARM64LabelInstruction(TR::InstOpCode::vgnop, node, sym, cond, precedingInstruction, cg),
        _site(site)
      {
      }

   virtual Kind getKind() { return IsVirtualGuardNOP; }

   void setSite(TR_VirtualGuardSite *site) { _site = site; }
   TR_VirtualGuardSite * getSite() { return _site; }

   virtual uint8_t *generateBinaryEncoding();
   virtual int32_t estimateBinaryLength(int32_t currentEstimate);
   virtual bool     isVirtualGuardNOPInstruction() {return true;}
   };
#endif

} // TR

#endif
