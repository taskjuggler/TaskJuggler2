/*
 * TaskScenario.h - TaskJuggler
 *
 * Copyright (c) 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _TaskScenario_h_
#define _TaskScenario_h_

#include <time.h>

#include <qptrlist.h>

#include "taskjuggler.h"
#include "ResourceList.h"

class Task;
class Resource;

class TaskScenario
{
    friend class Task;
    friend class TaskList;
public:
    TaskScenario();
    ~TaskScenario() { }
    
    void calcCompletionDegree(time_t now);

private:
    /// Pointer to the corresponding task.
    Task* task;

    /// Index of the scenario
    int index;
    
    /// Time when the task starts 
    time_t start;

    /// Time when the task ends
    time_t end;

    /* Specifies how many percent the task start can be delayed but still
     * finish in time if all goes well. This value is for documentation
     * purposes only. It is not used for task scheduling. */
    double startBuffer;

    /* Specifies how many percent the task can be finished earlier if
     * all goes well. This value is for documentation purposes only. It is
     * not used for task scheduling. */
    double endBuffer;
    
    /// Time when the start buffer ends.
    time_t startBufferEnd;

    /// Time when the end buffer starts.
    time_t endBufferStart;
    
    /// The duration of the task (in calendar days).
    double duration;

    /// The length of the task (in working days).
    double length;

    /// The effort of the task (in resource-days).
    double effort;

    /// List of booked resources.
    ResourceList bookedResources;

    /// Amount that is credited to the account at the start date.
    double startCredit;
    
    /// Amount that is credited to the account at the end date.
    double endCredit;

    /// User specified percentage of completion of the task
    int complete;

    /// Calculated completion degree
    double completionDegree;

    /// Status that the task is in (according to 'now' date)
    TaskStatus status;

    /// A longer description of the task status.
    QString statusNote;

    /// TRUE if the task has been completely scheduled.
    bool scheduled;
} ;

#endif

