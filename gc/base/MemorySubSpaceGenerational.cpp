/*******************************************************************************
 * Copyright IBM Corp. and others 1991
 *
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse Public License 2.0 which accompanies this
 * distribution and is available at https://www.eclipse.org/legal/epl-2.0/
 * or the Apache License, Version 2.0 which accompanies this distribution and
 * is available at https://www.apache.org/licenses/LICENSE-2.0.
 *
 * This Source Code may also be made available under the following
 * Secondary Licenses when the conditions for such availability set
 * forth in the Eclipse Public License, v. 2.0 are satisfied: GNU
 * General Public License, version 2 with the GNU Classpath
 * Exception [1] and GNU General Public License, version 2 with the
 * OpenJDK Assembly Exception [2].
 *
 * [1] https://www.gnu.org/software/classpath/license.html
 * [2] https://openjdk.org/legal/assembly-exception.html
 *
 * SPDX-License-Identifier: EPL-2.0 OR Apache-2.0 OR GPL-2.0-only WITH Classpath-exception-2.0 OR GPL-2.0-only WITH OpenJDK-assembly-exception-1.0
 *******************************************************************************/


#include "omrcfg.h"
#include "modronopt.h"

#if defined(OMR_GC_MODRON_SCAVENGER)

#include <string.h>

#include "MemorySubSpaceGenerational.hpp"

#include "AllocateDescription.hpp"
#include "Collector.hpp"
#include "Debug.hpp"
#include "EnvironmentBase.hpp"
#include "Heap.hpp"
#include "MemoryPool.hpp"
#include "MemorySpace.hpp" 
#include "MemorySubSpace.hpp"

/****************************************
 * Allocation
 ****************************************
 */

void *
MM_MemorySubSpaceGenerational::allocateObject(MM_EnvironmentBase *env, MM_AllocateDescription *allocDescription, MM_MemorySubSpace *baseSubSpace, MM_MemorySubSpace *previousSubSpace, bool shouldCollectOnFailure)
{
	Trc_MM_MSSGenerational_allocate_entry(env->getLanguageVMThread(), "Object", allocDescription->getBytesRequested(), this, getName(), baseSubSpace, previousSubSpace, (uintptr_t)shouldCollectOnFailure);

	if (shouldCollectOnFailure) {
		/* Should never receive this call */
		Trc_MM_MSSGenerational_allocate_exit(env->getLanguageVMThread(), "Object", allocDescription->getBytesRequested(), 1, NULL);
		return NULL;
	} else {
		if(previousSubSpace == _memorySubSpaceNew) {
			/* The allocate request is coming from new space - forward on to the old area */
			Trc_MM_MSSGenerational_allocate(env->getLanguageVMThread(), "Object", allocDescription->getBytesRequested(), _memorySubSpaceNew, _memorySubSpaceOld);
			void *result = _memorySubSpaceOld->allocateObject(env, allocDescription, baseSubSpace, this, shouldCollectOnFailure);
			Trc_MM_MSSGenerational_allocate_exit(env->getLanguageVMThread(), "Object", allocDescription->getBytesRequested(), 2, result);
			return result;
		}

		Trc_MM_MSSGenerational_allocate_exit(env->getLanguageVMThread(), "Object", allocDescription->getBytesRequested(), 3, NULL);
		/* The allocate comes from the old area - failure */
		return NULL;
	}
}

void *
MM_MemorySubSpaceGenerational::allocationRequestFailed(MM_EnvironmentBase *env, MM_AllocateDescription *allocateDescription, AllocationType allocationType, MM_ObjectAllocationInterface *objectAllocationInterface, MM_MemorySubSpace *baseSubSpace, MM_MemorySubSpace *previousSubSpace)
{

	Trc_MM_MSSGenerational_allocationRequestFailed_entry(env->getLanguageVMThread(), allocateDescription->getBytesRequested(), this, getName(), baseSubSpace, previousSubSpace, (uintptr_t)allocationType);

	/* TODO: This code is nearly the same as Flat and Concurrent - all three should be merged into a common superclass */
	void *addr = NULL;

	if (previousSubSpace == _memorySubSpaceNew) {
		/* Handle a failure coming from new space - attempt the old area before doing any collection work */
		Trc_MM_MSSGenerational_allocationRequestFailed1(env->getLanguageVMThread(), allocateDescription->getBytesRequested(), _memorySubSpaceNew, _memorySubSpaceOld);
		addr = _memorySubSpaceOld->allocationRequestFailed(env, allocateDescription, allocationType, objectAllocationInterface, baseSubSpace, this);
		if(NULL != addr) {
			Trc_MM_MSSGenerational_allocationRequestFailed_exit(env->getLanguageVMThread(), allocateDescription->getBytesRequested(), 1, addr);
			return addr;
		}
	}

	allocateDescription->saveObjects(env);
	if (!env->acquireExclusiveVMAccessForGC(_collector, true, true)) {
		allocateDescription->restoreObjects(env);
		Trc_MM_MSSGenerational_allocationRequestFailed(env->getLanguageVMThread(), allocateDescription->getBytesRequested(), 2);
		addr = allocateGeneric(env, allocateDescription, allocationType, objectAllocationInterface, baseSubSpace);
		if(NULL != addr) {
			Trc_MM_MSSGenerational_allocationRequestFailed_exit(env->getLanguageVMThread(), allocateDescription->getBytesRequested(), 2, addr);
			return addr;
		}

		if (!env->acquireExclusiveVMAccessForGC(_collector)) {
			allocateDescription->restoreObjects(env);
			Trc_MM_MSSGenerational_allocationRequestFailed(env->getLanguageVMThread(), allocateDescription->getBytesRequested(), 3);
			addr = allocateGeneric(env, allocateDescription, allocationType, objectAllocationInterface, baseSubSpace);
			if(NULL != addr) {
				/* Satisfied the allocate after having grabbed exclusive access to perform a GC (without actually performing the GC).  Raise
				 * an event for tracing / verbose to report the occurrence.
				 */
				reportAcquiredExclusiveToSatisfyAllocate(env, allocateDescription);
				Trc_MM_MSSGenerational_allocationRequestFailed_exit(env->getLanguageVMThread(), allocateDescription->getBytesRequested(), 3, addr);
				return addr;
			}

			reportAllocationFailureStart(env, allocateDescription);
			performResize(env, allocateDescription);
			Trc_MM_MSSGenerational_allocationRequestFailed(env->getLanguageVMThread(), allocateDescription->getBytesRequested(), 4);

			addr = allocateGeneric(env, allocateDescription, allocationType, objectAllocationInterface, baseSubSpace);

			if(NULL != addr) {
				/* Satisfied the allocate after having grabbed exclusive access to perform a GC (without actually performing the GC).  Raise
				 * an event for tracing / verbose to report the occurrence.
				 */
				reportAcquiredExclusiveToSatisfyAllocate(env, allocateDescription);
				reportAllocationFailureEnd(env);
				Trc_MM_MSSGenerational_allocationRequestFailed_exit(env->getLanguageVMThread(), allocateDescription->getBytesRequested(), 4, addr);
				return addr;
			}
			allocateDescription->saveObjects(env);
		} else {
			reportAllocationFailureStart(env, allocateDescription);
		}
	} else {
		reportAllocationFailureStart(env, allocateDescription);
	}

	Assert_MM_mustHaveExclusiveVMAccess(env->getOmrVMThread());

	allocateDescription->setAllocationType(allocationType);
	addr = _collector->garbageCollect(env, this, allocateDescription, J9MMCONSTANT_IMPLICIT_GC_DEFAULT, objectAllocationInterface, baseSubSpace, NULL);
	allocateDescription->restoreObjects(env);

	if(NULL != addr) {
		reportAllocationFailureEnd(env);
		Trc_MM_MSSGenerational_allocationRequestFailed_exit(env->getLanguageVMThread(), allocateDescription->getBytesRequested(), 5, addr);
		return addr;
	}

	/* A more aggressive collect here on failure */
	allocateDescription->saveObjects(env);
	addr = _collector->garbageCollect(env, this, allocateDescription, J9MMCONSTANT_IMPLICIT_GC_AGGRESSIVE, objectAllocationInterface, baseSubSpace, NULL);
	allocateDescription->restoreObjects(env);
	
	reportAllocationFailureEnd(env);
	Trc_MM_MSSGenerational_allocationRequestFailed_exit(env->getLanguageVMThread(), allocateDescription->getBytesRequested(), 6, addr);
	return addr;
}

void *
MM_MemorySubSpaceGenerational::allocateArrayletLeaf(MM_EnvironmentBase *env, MM_AllocateDescription *allocDescription, MM_MemorySubSpace *baseSubSpace, MM_MemorySubSpace *previousSubSpace, bool shouldCollectOnFailure)
{
	if (shouldCollectOnFailure) {
		/* Should never receive this call */
		return NULL;
	} else {
		if(previousSubSpace == _memorySubSpaceNew) {
			/* The allocate request is coming from new space - forward on to the old area */
			return _memorySubSpaceOld->allocateArrayletLeaf(env, allocDescription, baseSubSpace, this, shouldCollectOnFailure);
		}
	
		/* The allocate comes from the old area - failure */
		return NULL;
	}
}

void *
MM_MemorySubSpaceGenerational::allocateTLH(MM_EnvironmentBase *env, MM_AllocateDescription *allocDescription, MM_ObjectAllocationInterface *objectAllocationInterface, MM_MemorySubSpace *baseSubSpace, MM_MemorySubSpace *previousSubSpace, bool shouldCollectOnFailure)
{
	if (shouldCollectOnFailure) {
		/* Should never receive this call */
		Assert_MM_unreachable();
		return NULL;
	} else {
		if(previousSubSpace == _memorySubSpaceNew) {
			/* The allocate request is coming from new space - forward on to the old area */
			return _memorySubSpaceOld->allocateTLH(env, allocDescription, objectAllocationInterface, baseSubSpace, this, false);
		}
	
		/* The allocate comes from the old area - failure */
		return NULL;
	}
}

/****************************************
 * Internal Allocation
 ****************************************
 */
void
MM_MemorySubSpaceGenerational::abandonHeapChunk(void *addrBase, void *addrTop)
{
}

/****************************************
 * Sub Space Categorization
 ****************************************
 */
MM_MemorySubSpace *
MM_MemorySubSpaceGenerational::getDefaultMemorySubSpace()
{
	return _memorySubSpaceNew->getDefaultMemorySubSpace();
}

MM_MemorySubSpace *
MM_MemorySubSpaceGenerational::getTenureMemorySubSpace()
{
	return getMemorySubSpaceOld()->getDefaultMemorySubSpace();
}

/**
 * Initialization
 */
MM_MemorySubSpaceGenerational *
MM_MemorySubSpaceGenerational::newInstance(MM_EnvironmentBase *env, MM_MemorySubSpace *memorySubSpaceNew, MM_MemorySubSpace *memorySubSpaceOld, bool usesGlobalCollector, uintptr_t minimumSize, uintptr_t minimumSizeNew, uintptr_t initialSizeNew, uintptr_t maximumSizeNew, uintptr_t minimumSizeOld, uintptr_t initialSizeOld, uintptr_t maximumSizeOld, uintptr_t maximumSize)
{
	MM_MemorySubSpaceGenerational *memorySubSpace;
	
	memorySubSpace = (MM_MemorySubSpaceGenerational *)env->getForge()->allocate(sizeof(MM_MemorySubSpaceGenerational), OMR::GC::AllocationCategory::FIXED, OMR_GET_CALLSITE());
	if (memorySubSpace) {
		new(memorySubSpace) MM_MemorySubSpaceGenerational(env, memorySubSpaceNew, memorySubSpaceOld, usesGlobalCollector, minimumSize, minimumSizeNew, initialSizeNew, maximumSizeNew, minimumSizeOld, initialSizeOld, maximumSizeOld, maximumSize);
		if (!memorySubSpace->initialize(env)) {
			memorySubSpace->kill(env);
			memorySubSpace = NULL;
		}
	}
	return memorySubSpace;
}

bool
MM_MemorySubSpaceGenerational::initialize(MM_EnvironmentBase *env)
{
	if(!MM_MemorySubSpace::initialize(env)) {
		return false;
	}

	/* attach the children */
	registerMemorySubSpace(_memorySubSpaceOld);
	registerMemorySubSpace(_memorySubSpaceNew);
	
	return true;
}

void
MM_MemorySubSpaceGenerational::tearDown(MM_EnvironmentBase *env)
{
	MM_MemorySubSpace::tearDown(env);
}

/**
 * Perform the resize. For the generational, it applies only for the tenured subspace.
 * @return The actual amount resized.
 */
intptr_t
MM_MemorySubSpaceGenerational::performResize(MM_EnvironmentBase *env, MM_AllocateDescription *allocDescription)
{
	return getMemorySubSpaceOld()->performResize(env, allocDescription);
}

/**
 * Calculate the resize required (if any). For the generational, it applies only for the tenured subspace.
 * @return The actual amount resized.
 */

void
MM_MemorySubSpaceGenerational::checkResize(MM_EnvironmentBase *env, MM_AllocateDescription *allocDescription, bool _systemGC)
{
	getMemorySubSpaceOld()->checkResize(env, allocDescription, _systemGC);
	if (_extensions->isConcurrentScavengerEnabled()) {
		/* restore Nursery tilt */
		getMemorySubSpaceNew()->checkResize(env, allocDescription, _systemGC);
	}
}

/**
 * Get the size of heap available for contraction.
 * @return Size of heap available for contraction.
 */
uintptr_t 
MM_MemorySubSpaceGenerational::getAvailableContractionSize(MM_EnvironmentBase *env, MM_AllocateDescription *allocDescription)
{
	return getMemorySubSpaceOld()->getAvailableContractionSize(env, allocDescription);
}

/**
 * Counter balance a contract.
 * React to a pending contract of the the given subspace by possibly adjusted (expanding) other subspaces to fill minimum
 * quotas, etc.
 * The generational receiver determines which direction to push the request so that the opposing sibbling has a chance to
 * expand.
 * @return the adjusted contract size that is allowed to the receiver.
 */
uintptr_t
MM_MemorySubSpaceGenerational::counterBalanceContract(
	MM_EnvironmentBase *env,
	MM_MemorySubSpace *previousSubSpace,
	MM_MemorySubSpace *contractSubSpace,
	uintptr_t contractSize,
	uintptr_t contractAlignment)
{
	uintptr_t expandSize;
	/* Determine if a counter balancing expand is required */
	assume0(contractSize <= _currentSize);
	if((_currentSize - contractSize) >= _minimumSize) {
		return contractSize;
	}
	expandSize = _minimumSize - (_currentSize - contractSize);
	assume0(expandSize == MM_Math::roundToFloor(MM_GCExtensions::getExtensions(env)->heapAlignment, expandSize)); /* contract delta should be the same alignment as expand delta */
	
	/* Find the space that needs to expand, and do it */
	if(previousSubSpace == _memorySubSpaceNew) {
		return _memorySubSpaceOld->counterBalanceContractWithExpand(env, this, contractSubSpace, contractSize, contractAlignment, expandSize);
	}
	
	return _memorySubSpaceNew->counterBalanceContractWithExpand(env, this, contractSubSpace, contractSize, contractAlignment, expandSize);
}

uintptr_t
MM_MemorySubSpaceGenerational::releaseFreeMemoryPages(MM_EnvironmentBase* env, uintptr_t memoryType)
{
	Assert_MM_true(OMR_ARE_ALL_BITS_SET(memoryType, MEMORY_TYPE_OLD));

	uintptr_t releasedPages = _memorySubSpaceOld->releaseFreeMemoryPages(env);

	if (OMR_ARE_ALL_BITS_SET(memoryType, MEMORY_TYPE_NEW)) {
		releasedPages += _memorySubSpaceNew->releaseFreeMemoryPages(env);
	}

	return releasedPages;
}

#endif /* OMR_GC_MODRON_SCAVENGER */
