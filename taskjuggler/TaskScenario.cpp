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

#include "Resource.h"
#include "TaskScenario.h"
#include "Task.h"
#include "Project.h"

TaskScenario::TaskScenario()
{
    start = 0;
    end = 0;
    startBuffer = -1.0;
    endBuffer = -1.0;
    startBufferEnd = 0;
    endBufferStart = 0;
    duration = 0.0;
    length = 0.0;
    effort = 0.0;
    startCredit = -1.0;
    endCredit = -1.0;
    complete = -1;
    scheduled = FALSE;
}

void
TaskScenario::calcCompletionDegree(time_t now)
{
    if (now >= end)
    {
        completionDegree = 100.0;
        status = complete >= 0 && complete < 100 ? InProgressLate : Finished;
    }
    else if (now <= start)
    {
        completionDegree = 0.0;
        status = complete > 0 ? InProgressEarly : NotStarted;
    }
    else if (task->isContainer())
    {
        completionDegree = (100.0 / (end - start + 1)) * (now - start);
        status = InProgress;
    }   
    else 
    {
        status = OnTime;
        if (effort > 0.0)
        {
            completionDegree = (100.0 / effort) *
                task->getLoad(index, Interval(start, now)); 
        }
        else if (length > 0.0)
        {
            completionDegree = (100.0 / 
                task->getProject()->calcWorkingDays(Interval(start, end))) *
                task->getProject()->calcWorkingDays(Interval(start, now));
        }
        else
            completionDegree = (100.0 / (end - start + 1)) * (now - start);

        if (complete >= 0)
        {
            if (complete < completionDegree)
                status = InProgressLate;
            else if (complete > completionDegree)
                status = InProgressEarly;
        }
    }
}


