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

    void setStart(time_t s) { start = s; }
    void setEnd(time_t e) { end = e; }
    void setMaxEnd(time_t e) { maxEnd = e; }
    void setMinEnd(time_t e) { minEnd = e; }
    void setMaxStart(time_t s) { maxStart = s; }
    void setMinStart(time_t s) { minStart = s; }

    bool isStartOk() const
    {
        return !((minStart > 0 && minStart > start) ||
                 (maxStart > 0 && start > maxStart));
    }
    bool isEndOk() const
    {
        return !((minEnd > 0 && (end < minEnd)) ||
                 (maxEnd > 0 && (end > maxEnd)));
    }

    void calcCompletionDegree(time_t now);

    bool isDutyOf(const Resource* r) const
    {
        return bookedResources.containsRef((const CoreAttributes*) r) > 0;
    }

private:
    /// Pointer to the corresponding task.
    Task* task;

    /// Index of the scenario
    int index;
    
    /// Time when the task starts 
    time_t start;

    /// Time when the task ends
    time_t end;

    /// Ealiest day when the task should start
    time_t minStart;

    /// Latest day when the task should start
    time_t maxStart;

    /// Ealiest day when the task should end
    time_t minEnd;

    /// Latest day when the task should end
    time_t maxEnd;

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

    /// Amount that is credited to the account at the start date.
    double startCredit;
    
    /// Amount that is credited to the account at the end date.
    double endCredit;

    /// User specified percentage of completion of the task
    double reportedCompletion;

    /// Calculated completion degree
    double completionDegree;

    /// Status that the task is in (according to 'now' date)
    TaskStatus status;

    /// A longer description of the task status.
    QString statusNote;

    /// TRUE if the task has been completely scheduled.
    bool scheduled;
    
    /// List of booked resources.
    ResourceList bookedResources;
} ;

#endif

