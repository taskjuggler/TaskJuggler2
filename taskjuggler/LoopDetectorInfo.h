/*
 * LoopDetectorInfo.h - TaskJuggler
 *
 * Copyright (c) 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _LoopDetectorInfo_h_
#define _LoopDetectorInfo_h_

#include <qvaluelist.h>

class LoopDetectorInfo
{
public:
	LoopDetectorInfo() { }
	LoopDetectorInfo(Task* _t, bool ae) : t(_t), atEnd(ae) { }
	~LoopDetectorInfo() { }

	bool operator==(const LoopDetectorInfo& ldi) const
	{
		return t == ldi.t && atEnd == ldi.atEnd;
	}
	Task* getTask() const { return t; }
	bool getAtEnd() const { return atEnd; }
private:
	Task* t;
	bool atEnd;
} ;

class LDIList : public virtual QValueList<LoopDetectorInfo>
{
public:
	LDIList() { }
	~LDIList() { }
} ;

#endif

