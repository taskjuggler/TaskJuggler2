/*
 * TaskScenario.h - TaskJuggler
 *
 * Copyright (c) 2002 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "Resource.h"
#include "Interval.h"
#include "TaskScenario.h"
#include "Task.h"
#include "ResourceTreeIterator.h"
#include "Project.h"

TaskScenario::TaskScenario()
{
    start = specifiedStart = 0;
    end = specifiedEnd = 0;
    specifiedScheduled = FALSE;
    startBuffer = -1.0;
    endBuffer = -1.0;
    startBufferEnd = 0;
    endBufferStart = 0;
    duration = 0.0;
    length = 0.0;
    effort = 0.0;
    startCredit = -1.0;
    endCredit = -1.0;
    reportedCompletion = -1.0;
    containerCompletion = -1.0;
    completionDegree = 0.0;
    scheduled = FALSE;
    criticalness = pathCriticalness = 0.0;
    startCanBeDetermined = false;
    endCanBeDetermined = false;
}

void
TaskScenario::calcCompletionDegree(time_t now)
{
    if (now > end)
    {
        completionDegree = 100.0;
        status = reportedCompletion >= 0 && reportedCompletion < 100 ?
            Late : Finished;
    }
    else if (now <= start)
    {
        completionDegree = 0.0;
        status = reportedCompletion > 0 ? InProgressEarly : NotStarted;
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

        if (reportedCompletion >= 0.0)
        {
            if (reportedCompletion < completionDegree)
                status = InProgressLate;
            else if (reportedCompletion > completionDegree)
                status = InProgressEarly;
        }
    }
}

bool TaskScenario::isDutyOf(const Resource* r) const
{
    for (ConstResourceTreeIterator rti(r); *rti; ++rti)
        if (bookedResources.containsRef((const CoreAttributes*) *rti) > 0)
            return true;

    return false;
}
