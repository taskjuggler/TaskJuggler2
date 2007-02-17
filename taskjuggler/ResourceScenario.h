/*
 * ResourceScenario.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _ResourceScenario_h_
#define _ResourceScenario_h_

#include "TaskList.h"
#include "Task.h"

class ResourceScenario
{
    friend class Resource;

public:
    ResourceScenario() :
        allocatedTasks(),
        firstSlot(-1),
        lastSlot(-1)
    { }

    ~ResourceScenario() { }

    void addTask(const Task* t)
    {
        if (allocatedTasks.findRef(t) < 0)
            allocatedTasks.append(t);
    }
private:
    TaskList allocatedTasks;
    int firstSlot;
    int lastSlot;
} ;

#endif

