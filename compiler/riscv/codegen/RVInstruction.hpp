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

#ifndef RVINSTRUCTION_INCL
#define RVINSTRUCTION_INCL



#include "codegen/CodeGenerator.hpp"
#include "codegen/Instruction.hpp"
#include "codegen/RVInstructionUtils.hpp"
#include "codegen/MemoryReference.hpp"
#include "il/LabelSymbol.hpp"
#include "infra/Assert.hpp"

class TR_VirtualGuardSite;
namespace TR { class SymbolReference; }

namespace TR
{

class RtypeInstruction : public TR::Instruction
   {
   TR::Register *_target1Register;
   TR::Register *_source1Register;
   TR::Register *_source2Register;

   public:

   RtypeInstruction(TR::InstOpCode::Mnemonic op,
         TR::Node          *n,
         TR::Register      *treg,
         TR::Register      *s1reg,
         TR::Register      *s2reg,
         TR::CodeGenerator *cg)
      : TR::Instruction(op, n, cg),
        _target1Register(treg),
        _source1Register(s1reg),
        _source2Register(s2reg)
      {
      useRegister(treg);
      useRegister(s1reg);
      useRegister(s2reg);
      }

   RtypeInstruction(TR::InstOpCode::Mnemonic op,
         TR::Node          *n,
         TR::Register      *treg,
         TR::Register      *s1reg,
         TR::Register      *s2reg,
         TR::Instruction   *precedingInstruction,
         TR::CodeGenerator *cg)
      : TR::Instruction(op, n, precedingInstruction, cg),
        _target1Register(treg),
        _source1Register(s1reg),
        _source2Register(s2reg)
      {
      useRegister(treg);
      useRegister(s1reg);
      useRegister(s2reg);
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsRTYPE; }

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
    * @brief Gets source register 1
    * @return source register
    */
   TR::Register *getSource1Register() {return _source1Register;}
   /**
    * @brief Sets source register 1
    * @param[in] sr : source register
    * @return source register
    */
   TR::Register *setSource1Register(TR::Register *sr) {return (_source1Register = sr);}

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



class ItypeInstruction : public TR::Instruction
   {
   TR::Register *_target1Register;
   TR::Register *_source1Register;
   uint32_t _imm;

   public:

   ItypeInstruction(TR::InstOpCode::Mnemonic op,
         TR::Node          *n,
         TR::Register      *treg,
         TR::Register      *s1reg,
         uint32_t          imm,
         TR::CodeGenerator *cg)

      : TR::Instruction(op, n, cg),
        _target1Register(treg),
        _source1Register(s1reg),
        _imm(imm)
      {
      useRegister(treg);
      useRegister(s1reg);
      }

   ItypeInstruction(TR::InstOpCode::Mnemonic op,
         TR::Node          *n,
         TR::Register      *treg,
         TR::Register      *s1reg,
         uint32_t          imm,
         TR::Instruction   *precedingInstruction,
         TR::CodeGenerator *cg)
      : TR::Instruction(op, n, precedingInstruction, cg),
        _target1Register(treg),
        _source1Register(s1reg),
        _imm(imm)
      {
      useRegister(treg);
      useRegister(s1reg);
      }

   ItypeInstruction(TR::InstOpCode::Mnemonic op,
            TR::Node          *n,
            TR::Register      *treg,
            TR::Register      *s1reg,
            uint32_t          imm,
            TR::RegisterDependencyConditions *cond,
            TR::CodeGenerator *cg)
      : TR::Instruction(op, n, cond, cg),
        _target1Register(treg),
        _source1Register(s1reg),
        _imm(imm)
      {
      useRegister(treg);
      useRegister(s1reg);
      }

   ItypeInstruction(TR::InstOpCode::Mnemonic op,
            TR::Node          *n,
            TR::Register      *treg,
            TR::Register      *s1reg,
            uint32_t          imm,
            TR::RegisterDependencyConditions *cond,
            TR::Instruction   *precedingInstruction,
            TR::CodeGenerator *cg)
      : TR::Instruction(op, n, cond, precedingInstruction, cg),
        _target1Register(treg),
        _source1Register(s1reg),
        _imm(imm)
      {
      useRegister(treg);
      useRegister(s1reg);
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsITYPE; }

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
    * @brief Gets source register 1
    * @return source register
    */
   TR::Register *getSource1Register() {return _source1Register;}
   /**
    * @brief Sets source register 1
    * @param[in] sr : source register
    * @return source register
    */
   TR::Register *setSource1Register(TR::Register *sr) {return (_source1Register = sr);}

   virtual uint32_t getSourceImmediate()            {return _imm;}

   virtual uint32_t setSourceImmediate(uint32_t si) {return (_imm = si);}

   /**
    * @brief Gets i-th source register
    * @param[in] i : index
    * @return i-th source register or NULL
    */
   virtual TR::Register *getSourceRegister(uint32_t i) {if      (i==0) return getSource1Register();
                                               return NULL;}

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


class LoadInstruction : public TR::Instruction
   {
   TR::Register *_target1Register;
   TR::MemoryReference *_memoryReference;

   public:

   LoadInstruction(TR::InstOpCode::Mnemonic op,
         TR::Node          *n,
         TR::Register      *treg,
         TR::MemoryReference *mr,
         TR::CodeGenerator *cg)
      : TR::Instruction(op, n, cg),
        _target1Register(treg),
        _memoryReference(mr)
      {
      useRegister(treg);
      mr->bookKeepingRegisterUses(this, cg);
      }

   LoadInstruction(TR::InstOpCode::Mnemonic op,
         TR::Node          *n,
         TR::Register      *treg,
         TR::MemoryReference *mr,
         TR::Instruction   *precedingInstruction,
         TR::CodeGenerator *cg)
      : TR::Instruction(op, n, precedingInstruction, cg),
        _target1Register(treg),
        _memoryReference(mr)
      {
      useRegister(treg);
      mr->bookKeepingRegisterUses(this, cg);
      // TODO: why incRegisterTotalUseCounts() here but not above?
      // This is how it's done in Aarch64. but could be wrong. Investigate.
      //mr->incRegisterTotalUseCounts(cg);
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsLOAD; }

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
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();

   };


class StypeInstruction : public TR::Instruction
   {
   TR::Register *_source1Register;
   TR::Register *_source2Register;
   uint32_t _imm;

   public:

   StypeInstruction(TR::InstOpCode::Mnemonic op,
         TR::Node          *n,
         TR::Register      *s1reg,
         TR::Register      *s2reg,
         uint32_t          imm,
         TR::CodeGenerator *cg)
      : TR::Instruction(op, n, cg),
        _source1Register(s1reg),
        _source2Register(s2reg),
        _imm(imm)
      {
      useRegister(s1reg);
      useRegister(s2reg);
      }

   StypeInstruction(TR::InstOpCode::Mnemonic op,
         TR::Node          *n,
         TR::Register      *s1reg,
         TR::Register      *s2reg,
         uint32_t          imm,
         TR::Instruction   *precedingInstruction,
         TR::CodeGenerator *cg)
      : TR::Instruction(op, n, precedingInstruction, cg),
        _source1Register(s1reg),
        _source2Register(s2reg),
        _imm(imm)
      {
      useRegister(s1reg);
      useRegister(s2reg);
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsSTYPE; }

   /**
    * @brief Gets source register 1
    * @return source register
    */
   TR::Register *getSource1Register() {return _source1Register;}
   /**
    * @brief Sets source register 1
    * @param[in] sr : source register
    * @return source register
    */
   TR::Register *setSource1Register(TR::Register *sr) {return (_source1Register = sr);}

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

   virtual uint32_t getSourceImmediate()            {return _imm;}

   virtual uint32_t setSourceImmediate(uint32_t si) {return (_imm = si);}

   /**
    * @brief Gets i-th source register
    * @param[in] i : index
    * @return i-th source register or NULL
    */
   virtual TR::Register *getSourceRegister(uint32_t i) {if      (i==0) return getSource1Register();
                                               else if (i==1) return _source2Register; return NULL;}

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

class StoreInstruction : public TR::Instruction
   {
   TR::Register *_source1Register;
   TR::MemoryReference *_memoryReference;

   public:

   StoreInstruction(TR::InstOpCode::Mnemonic op,
         TR::Node          *n,
         TR::MemoryReference *mr,
         TR::Register      *sreg,
         TR::CodeGenerator *cg)
      : TR::Instruction(op, n, cg),
        _source1Register(sreg),
        _memoryReference(mr)
      {
      useRegister(sreg);
      mr->bookKeepingRegisterUses(this, cg);
      }

   StoreInstruction(TR::InstOpCode::Mnemonic op,
         TR::Node          *n,
         TR::MemoryReference *mr,
         TR::Register      *sreg,
         TR::Instruction   *precedingInstruction,
         TR::CodeGenerator *cg)
      : TR::Instruction(op, n, precedingInstruction, cg),
        _source1Register(sreg),
        _memoryReference(mr)
      {
      useRegister(sreg);
      mr->bookKeepingRegisterUses(this, cg);
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsSTORE; }

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
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();

   };

class BtypeInstruction : public StypeInstruction
   {
   TR::LabelSymbol *_symbol;
   int32_t _estimatedBinaryLocation;

   public:
     BtypeInstruction(
       TR::InstOpCode::Mnemonic op,
       TR::Node                 *n,
       TR::LabelSymbol          *sym,
       TR::Register             *s1reg,
       TR::Register             *s2reg,
       TR::CodeGenerator        *cg
       )
     : StypeInstruction(op, n, s1reg, s2reg, 0, cg),
       _symbol(sym),
       _estimatedBinaryLocation(0)
     {
     }

     BtypeInstruction(
       TR::InstOpCode::Mnemonic op,
       TR::Node                 *n,
       TR::LabelSymbol          *sym,
       TR::Register             *s1reg,
       TR::Register             *s2reg,
       TR::Instruction          *precedingInstruction,
       TR::CodeGenerator        *cg
       )
     : StypeInstruction(op, n, s1reg, s2reg, 0, precedingInstruction, cg),
       _symbol(sym),
       _estimatedBinaryLocation(0)
     {
     }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsBTYPE; }

   uint32_t setSourceImmediate(uint32_t si)
      {
      TR_ASSERT(false, "Should not be used with B-type instructions, use setLabelSymbol()!");
      return 0;
      }

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

   virtual TR::BtypeInstruction *getBtypeInstruction();

   /**
    * @brief Expands conditional branch to 'far' branch.
    *
    * This is called after binary encoding length estimation for branches whose
    * target is outside of B-type instruction immediate.
    */
   void expandIntoFarBranch();
   };

class UtypeInstruction : public TR::Instruction
   {
   TR::Register *_target1Register;
   uint32_t _imm;

   public:

   UtypeInstruction(TR::InstOpCode::Mnemonic op,
         TR::Node          *n,
         uint32_t          imm,
         TR::Register      *treg,
         TR::CodeGenerator *cg)
      : TR::Instruction(op, n, cg),
        _target1Register(treg),
        _imm(imm)
      {
      useRegister(treg);
      }

   UtypeInstruction(TR::InstOpCode::Mnemonic op,
         TR::Node          *n,
         uint32_t          imm,
         TR::Register      *treg,
         TR::RegisterDependencyConditions *cond,
         TR::CodeGenerator *cg)
      : TR::Instruction(op, n, cond, cg),
        _target1Register(treg),
        _imm(imm)
      {
      useRegister(treg);
      }

   UtypeInstruction(TR::InstOpCode::Mnemonic op,
         TR::Node          *n,
         uint32_t          imm,
         TR::Register      *treg,
         TR::Instruction   *precedingInstruction,
         TR::CodeGenerator *cg)
      : TR::Instruction(op, n, precedingInstruction, cg),
        _target1Register(treg),
        _imm(imm)
      {
      useRegister(treg);
      }

   UtypeInstruction(TR::InstOpCode::Mnemonic op,
         TR::Node          *n,
         uint32_t          imm,
         TR::Register      *treg,
         TR::RegisterDependencyConditions *cond,
         TR::Instruction   *precedingInstruction,
         TR::CodeGenerator *cg)
      : TR::Instruction(op, n, cond, precedingInstruction, cg),
        _target1Register(treg),
        _imm(imm)
      {
      useRegister(treg);
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsUTYPE; }

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

   virtual uint32_t getSourceImmediate()            {return _imm;}

   virtual uint32_t setSourceImmediate(uint32_t si) {return (_imm = si);}

   /**
    * @brief Gets i-th source register
    * @param[in] i : index
    * @return i-th source register or NULL
    */
   virtual TR::Register *getSourceRegister(uint32_t i) {return NULL;}

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

class JtypeInstruction : public UtypeInstruction
   {
   // Only one of the following can be used at time!
   TR::SymbolReference *_symbolReference;
   TR::LabelSymbol *_symbol;
   TR::Snippet *_snippet;

   public:

   JtypeInstruction(TR::InstOpCode::Mnemonic op,
         TR::Node          *n,
         TR::Register      *treg,
         uintptr_t        imm,
         TR::RegisterDependencyConditions *cond,
         TR::SymbolReference *sr,
         TR::Snippet       *s,
         TR::CodeGenerator *cg)
      : UtypeInstruction(op, n, 0, treg, cond, cg),
        _symbolReference(sr),
        _symbol(nullptr),
        _snippet(s)
      {
      }

   JtypeInstruction(TR::InstOpCode::Mnemonic op,
         TR::Node          *n,
         TR::Register      *treg,
         uintptr_t        imm,
         TR::RegisterDependencyConditions *cond,
         TR::SymbolReference *sr,
         TR::Snippet       *s,
         TR::Instruction   *precedingInstruction,
         TR::CodeGenerator *cg)
      : UtypeInstruction(op, n, 0, treg, cond, precedingInstruction, cg),
        _symbolReference(sr),
        _symbol(nullptr),
        _snippet(s)
      {
      }

   JtypeInstruction(TR::InstOpCode::Mnemonic op,
         TR::Node          *n,
         TR::Register      *treg,
         TR::LabelSymbol   *label,
         TR::CodeGenerator *cg)

      : UtypeInstruction(op, n, 0, treg, cg),
        _symbolReference(nullptr),
        _symbol(label)
      {
      }

   JtypeInstruction(TR::InstOpCode::Mnemonic op,
         TR::Node          *n,
         TR::Register      *treg,
         TR::LabelSymbol   *label,
         TR::RegisterDependencyConditions *cond,
         TR::CodeGenerator *cg)

      : UtypeInstruction(op, n, 0, treg, cond, cg),
        _symbolReference(nullptr),
        _symbol(label)
      {
      }

   JtypeInstruction(TR::InstOpCode::Mnemonic op,
         TR::Node          *n,
         TR::Register      *treg,
         TR::LabelSymbol   *label,
         TR::Snippet       *snippet,
         TR::RegisterDependencyConditions *cond,
         TR::CodeGenerator *cg)

      : UtypeInstruction(op, n, 0, treg, cond, cg),
        _symbolReference(nullptr),
        _symbol(label),
        _snippet(snippet)
      {
      }

   JtypeInstruction(TR::InstOpCode::Mnemonic op,
         TR::Node          *n,
         TR::Register      *treg,
         TR::LabelSymbol   *label,
         TR::Instruction   *precedingInstruction,
         TR::CodeGenerator *cg)
      : UtypeInstruction(op, n, 0, treg, precedingInstruction, cg),
        _symbolReference(nullptr),
        _symbol(label)
      {
      }

   JtypeInstruction(TR::InstOpCode::Mnemonic op,
         TR::Node          *n,
         TR::Register      *treg,
         TR::LabelSymbol   *label,
         TR::RegisterDependencyConditions *cond,
         TR::Instruction   *precedingInstruction,
         TR::CodeGenerator *cg)

      : UtypeInstruction(op, n, 0, treg, cond, precedingInstruction, cg),
        _symbolReference(nullptr),
        _symbol(label)
      {
      }

   JtypeInstruction(TR::InstOpCode::Mnemonic op,
         TR::Node          *n,
         TR::Register      *treg,
         TR::LabelSymbol   *label,
         TR::Snippet       *snippet,
         TR::RegisterDependencyConditions *cond,
         TR::Instruction   *precedingInstruction,
         TR::CodeGenerator *cg)

      : UtypeInstruction(op, n, 0, treg, cond, precedingInstruction, cg),
        _symbolReference(nullptr),
        _symbol(label),
        _snippet(snippet)
      {
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsJTYPE; }

   /**
    * @brief Gets label symbol
    * @return label symbol
    */
   TR::LabelSymbol *getLabelSymbol() {return _symbol;}

   /**
    * @brief Gets symbol reference
    * @return symbol reference
    */
   TR::SymbolReference *getSymbolReference() {return _symbolReference;}

   virtual uint32_t setSourceImmediate(uint32_t si)
      {
      TR_ASSERT(false, "Should not be used with J-type instructions, use setLabelSymbol()!");
      return 0;
      }

   /**
    * @brief Gets snippet associated with this instruction (if any).
    *
    * @return snippet
    */
   TR::Snippet *getSnippet() { return _snippet;}

   /**
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();
   };

class LabelInstruction : public TR::Instruction
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
   LabelInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::LabelSymbol *sym, TR::CodeGenerator *cg)
      : TR::Instruction(op, node, cg), _symbol(sym)
      {
      TR_ASSERT(op == TR::InstOpCode::label, "Invalid opcode for label instruction, must be ::label");
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
   LabelInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::LabelSymbol *sym,
                         TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : TR::Instruction(op, node, precedingInstruction, cg), _symbol(sym)
      {
      TR_ASSERT(op == TR::InstOpCode::label, "Invalid opcode for label instruction, must be ::label");
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
   LabelInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::LabelSymbol *sym,
                         TR::RegisterDependencyConditions *cond, TR::CodeGenerator *cg)
      : TR::Instruction(op, node, cond, cg), _symbol(sym)
      {
      TR_ASSERT(op == TR::InstOpCode::label, "Invalid opcode for label instruction, must be ::label");
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
   LabelInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::LabelSymbol *sym,
                         TR::RegisterDependencyConditions *cond,
                         TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : TR::Instruction(op, node, cond, precedingInstruction, cg), _symbol(sym)
      {
      TR_ASSERT(op == TR::InstOpCode::label, "Invalid opcode for label instruction, must be ::label");
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

class AdminInstruction : public TR::Instruction
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
   AdminInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Node *fenceNode, TR::CodeGenerator *cg)
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
   AdminInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, TR::Node *fenceNode,
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
   AdminInstruction(TR::InstOpCode::Mnemonic op, TR::RegisterDependencyConditions *cond,
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
   AdminInstruction(TR::InstOpCode::Mnemonic op, TR::RegisterDependencyConditions *cond,
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

class DataInstruction : public TR::Instruction
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
   DataInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, uint32_t imm, TR::CodeGenerator *cg)
      : TR::Instruction(op, node, cg), _sourceImmediate(imm)
      {
      TR_ASSERT_FATAL_WITH_NODE(node, op == TR::InstOpCode::dd, "Opcode of data instruction is not dd");
      }
   /*
    * @brief Constructor
    * @param[in] op : instruction opcode
    * @param[in] node : node
    * @param[in] imm : immediate value
    * @param[in] precedingInstruction : preceding instruction
    * @param[in] cg : CodeGenerator
    */
   DataInstruction(TR::InstOpCode::Mnemonic op, TR::Node *node, uint32_t imm,
                       TR::Instruction *precedingInstruction, TR::CodeGenerator *cg)
      : TR::Instruction(op, node, precedingInstruction, cg), _sourceImmediate(imm)
      {
      TR_ASSERT_FATAL_WITH_NODE(node, op == TR::InstOpCode::dd, "Opcode of data instruction is not dd");
      }

   /**
    * @brief Gets instruction kind
    * @return instruction kind
    */
   virtual Kind getKind() { return IsData; }

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
    * @brief Generates binary encoding of the instruction
    * @return instruction cursor
    */
   virtual uint8_t *generateBinaryEncoding();
   };

#ifdef J9_PROJECT_SPECIFIC
class VGNOPInstruction : public TR::LabelInstruction
   {
   private:
   TR_VirtualGuardSite *_site;

   public:
   VGNOPInstruction(TR::Node                       *node,
                                 TR_VirtualGuardSite              *site,
                                 TR::RegisterDependencyConditions *cond,
                                 TR::LabelSymbol                  *sym,
                                 TR::CodeGenerator                *cg)
      : TR::LabelInstruction(TR::InstOpCode::vgnop, node, sym, cond, cg),
        _site(site)
      {
      }

   VGNOPInstruction(TR::Node                       *node,
                                 TR_VirtualGuardSite              *site,
                                 TR::RegisterDependencyConditions *cond,
                                 TR::LabelSymbol                  *sym,
                                 TR::Instruction                  *precedingInstruction,
                                 TR::CodeGenerator                *cg)
      : TR::LabelInstruction(TR::InstOpCode::vgnop, node, sym, cond, precedingInstruction, cg),
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



} // namespace TR

#endif // RVINSTRUCTION_INCL
