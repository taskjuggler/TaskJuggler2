/*
 * ParserTreeContext.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _ParserTreeContext_h_
#define _ParserTreeContext_h_

class Task;
class Resource;
class TaskScenario;

class ParserTreeContext
{
public:
    ParserTreeContext() { }
    ~ParserTreeContext() { }

    void setParentTask(Task* t) { u.parentTask = 0; }
    Task* getParentTask() { return u.parentTask; }

    void setTaskScenario(TaskScenario* ts) { u.taskScenario = 0; }
    TaskScenario* getTaskScenario() { return u.taskScenario; }
    
private:
    union
    {
        Task* parentTask;
        Resource* parentResource;
        TaskScenario* taskScenario;
    } u;
} ;

#endif

