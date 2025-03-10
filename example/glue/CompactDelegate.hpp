/*******************************************************************************
 * Copyright IBM Corp. and others 2017
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

#ifndef COMPACTDELEGATE_HPP_
#define COMPACTDELEGATE_HPP_

#include "omrcfg.h"
#include "omrgcconsts.h"

class MM_CompactScheme;
class MM_EnvironmentBase;
class MM_MarkMap;

#if defined(OMR_GC_MODRON_COMPACTION)

/**
 * Delegate class provides implementations for methods required for Collector Language Interface
 */
class MM_CompactDelegate
{
	/*
	 * Data members
	 */
private:
	OMR_VM *_omrVM;
	MM_CompactScheme *_compactScheme;
	MM_MarkMap *_markMap;

protected:

public:

	/*
	 * Function members
	 */
private:

protected:

public:
	/**
	 * Initialize the delegate.
	 *
	 * @param omrVM
	 * @return true if delegate initialized successfully
	 */
	void
	tearDown(MM_EnvironmentBase *env) { }

	bool
	initialize(MM_EnvironmentBase *env, OMR_VM *omrVM, MM_MarkMap *markMap, MM_CompactScheme *compactScheme)
	{
		_omrVM  = omrVM;
		_compactScheme = compactScheme;
		_markMap = markMap;
		return true;
	}

	void
	verifyHeap(MM_EnvironmentBase *env, MM_MarkMap *markMap) { }

	void
	fixupRoots(MM_EnvironmentBase *env, MM_CompactScheme *compactScheme) { }

	void
	workerCleanupAfterGC(MM_EnvironmentBase *env) { }

	void
	mainSetupForGC(MM_EnvironmentBase *env) { }

	MM_CompactDelegate()
		: _compactScheme(NULL)
		, _markMap(NULL)
		, _omrVM(NULL)
	{}
};

#endif /* OMR_GC_MODRON_COMPACTION */
#endif /* COMPACTDELEGATE_HPP_ */
