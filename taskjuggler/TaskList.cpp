/*
 * TaskList.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#include "TaskList.h"
#include "Task.h"
#include "Resource.h"

Task*
TaskList::getTask(const QString& id) const
{
	for (TaskListIterator tli(*this); *tli != 0; ++tli)
		if ((*tli)->getId() == id)
			return *tli;

	return 0;
}

bool
TaskList::isSupportedSortingCriteria(int sc)
{
	switch (sc & 0xFFFF)
	{
	case TreeMode:
	case StartUp:
	case StartDown:
	case EndUp:
	case EndDown:
	case StatusUp:
	case StatusDown:
	case CompletedUp:
	case CompletedDown:
	case PrioUp:
	case PrioDown:
	case ResponsibleUp:
	case ResponsibleDown:
		return TRUE;
	default:
		return CoreAttributesList::isSupportedSortingCriteria(sc);
	}		
}

int
TaskList::compareItemsLevel(Task* t1, Task* t2, int level)
{
	if (level < 0 || level >= maxSortingLevel)
		return -1;

	int sc = sorting[level] >> 16;
	switch (sorting[level] & 0xFFFF)
	{
	case TreeMode:
		if (level == 0)
			return compareTreeItemsT(this, t1, t2);
		else
			return t1->getSequenceNo() == t2->getSequenceNo() ? 0 :
				t1->getSequenceNo() < t2->getSequenceNo() ? -1 : 1;
	case StartUp:
		return t1->scenarios[sc].start == t2->scenarios[sc].start ? 0 :
			t1->scenarios[sc].start < t2->scenarios[sc].start ? -1 : 1;
	case StartDown:
		return t1->scenarios[sc].start == t2->scenarios[sc].start ? 0 :
			t1->scenarios[sc].start > t2->scenarios[sc].start ? -1 : 1;
	case EndUp:
		return t1->scenarios[sc].end == t2->scenarios[sc].end ? 0 :
			t1->scenarios[sc].end < t2->scenarios[sc].end ? -1 : 1;
	case EndDown:
		return t1->scenarios[sc].end == t2->scenarios[sc].end ? 0 :
			t1->scenarios[sc].end > t2->scenarios[sc].end ? -1 : 1;
	case StatusUp:
		return t1->scenarios[sc].status == t2->scenarios[sc].status ? 0 :
			t1->scenarios[sc].status < t2->scenarios[sc].status ? -1 : 1;
	case StatusDown:
		return t1->scenarios[sc].status == t2->scenarios[sc].status ? 0 :
			t1->scenarios[sc].status > t2->scenarios[sc].status ? -1 : 1;
	case CompletedUp:
    {
        /* Unfortunately the floating point arithmetic on x86 processors is
         * slightly different from other processors. To get identical
         * results on all CPUs we ignore some precision that we don't need
         * anyhow. */
        int cd1 = (int) (t1->getCompletionDegree(sc) * 1000);
        int cd2 = (int) (t2->getCompletionDegree(sc) * 1000);
        return cd1 == cd2 ? 0 : cd1 < cd2 ? -1 : 1;
    }
	case CompletedDown:
    {
        int cd1 = (int) (t1->getCompletionDegree(sc) * 1000);
        int cd2 = (int) (t2->getCompletionDegree(sc) * 1000);
        return cd1 == cd2 ? 0 : cd1 > cd2 ? -1 : 1;
    }
	case PrioUp:
		if (t1->priority == t2->priority)
			return 0;
		else
			return (t1->priority - t2->priority);
	case PrioDown:
		if (t1->priority == t2->priority)
			return 0;
		else
			return (t2->priority - t1->priority);
	case ResponsibleUp:
	{
		QString fn1;
		t1->responsible->getFullName(fn1);
		QString fn2;
		t2->responsible->getFullName(fn2);
		return - fn1.compare(fn2);
	}
	case ResponsibleDown:
	{
		QString fn1;
		t1->responsible->getFullName(fn1);
		QString fn2;
		t2->responsible->getFullName(fn2);
		return fn1.compare(fn2);
	}
	default:
		return CoreAttributesList::compareItemsLevel(t1, t2, level);
	}		
}

int
TaskList::compareItems(QCollection::Item i1, QCollection::Item i2)
{
	Task* t1 = static_cast<Task*>(i1);
	Task* t2 = static_cast<Task*>(i2);

	int res;
	for (int i = 0; i < CoreAttributesList::maxSortingLevel; ++i)
		if ((res = compareItemsLevel(t1, t2, i)) != 0)
			return res;
	return res;
}


