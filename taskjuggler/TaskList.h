/*
 * TaskList.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef _TaskList_h_
#define _TaskList_h_

#include "CoreAttributesList.h"

class QString;
class Task;

/**
 * @short The class stores a list of tasks.
 * @see Task 
 * @author Chris Schlaeger <cs@suse.de>
 */
class TaskList : public virtual CoreAttributesList
{
public:
    TaskList()
    {
        sorting[0] = CoreAttributesList::TreeMode;
        sorting[1] = CoreAttributesList::StartUp;
        sorting[2] = CoreAttributesList::EndUp;
    }
    virtual ~TaskList() { }

    Task* getTask(const QString& id) const;

    static bool isSupportedSortingCriteria(int sc);
    
    virtual int compareItemsLevel(Task* t1, Task* T2, int level);

protected:
    virtual int compareItems(QCollection::Item i1, QCollection::Item i2);
} ;

/**
 * @short Iterator class for TaskList objects.
 * @see TaskList
 * @author Chris Schlaeger <cs@suse.de>
 */
class TaskListIterator : public virtual CoreAttributesListIterator 
{
public:
    TaskListIterator(const CoreAttributesList& l) :
        CoreAttributesListIterator(l) { }
    virtual ~TaskListIterator() { }
    Task* operator*() { return (Task*) get(); }
} ;

#endif

