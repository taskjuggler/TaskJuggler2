/*
 * ICalReport.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@kde.org>
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Klaas Freitag <freitag@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <config.h>

#ifdef HAVE_KDE

#include "ICalReport.h"

#include <qptrdict.h>
#include <qfile.h>
#include <klocale.h>
#include <libkcal/icalformat.h>

#include "Project.h"
#include "Utility.h"
#include "ExpressionTree.h"
#include "Task.h"
#include "Resource.h"
#include "Operation.h"

// Older versions of KDE do not have this macro
#ifndef KDE_IS_VERSION
#define KDE_IS_VERSION(a,b,c) 0
#endif

ICalReport::ICalReport(Project* p, const QString& file, const QString& defFile,
                       int dl) : Report(p, file, defFile, dl)
{
    taskSortCriteria[0] = CoreAttributesList::TreeMode;
    taskSortCriteria[1] = CoreAttributesList::StartUp;
    taskSortCriteria[2] = CoreAttributesList::EndUp;
    resourceSortCriteria[0] = CoreAttributesList::TreeMode;
    resourceSortCriteria[1] = CoreAttributesList::IdUp;

    // Use the fist scenario as default.
    scenarios.append(0);

    // show all tasks
    hideTask = new ExpressionTree(new Operation(0));
    // show all resources
    hideResource = new ExpressionTree(new Operation(0));
}

KCal::Todo*
ICalReport::generateTODO(Task* task, ResourceList& resourceList)
{
    KCal::Todo *todo = new KCal::Todo();

    QDateTime dt;

    /* Start-Time of the task */
    dt.setTime_t(task->getStart(scenarios[0]));
    todo->setDtStart(dt);
    todo->setHasDueDate(TRUE);

    /* Due-Time of the todo -> plan End  */
    dt.setTime_t(task->getEnd(scenarios[0]) + 1);
    todo->setDtDue(dt);
    todo->setHasStartDate(TRUE);

    // Make sure that the time is not ignored.
    todo->setFloats(FALSE);

    /* Description and summary -> project ID */
    todo->setDescription(task->getNote());
    todo->setSummary(task->getName());

    /* ICal defines priorities between 1..9 where 1 is the highest. TaskJuggler
     * uses Priority 1 - 1000, 1000 being the highest. So we have to map the
     * priorities. */
    todo->setPriority(1 + ((1000 - task->getPriority()) / 100));

    todo->setCompleted(task->getCalcedCompletionDegree(scenarios[0]));

    /* Resources */
    QStringList strList;
    ResourceListIterator rli = task->getBookedResourcesIterator(scenarios[0]);
    for (; *rli != 0; ++rli)
    {
        // We only include resources that have not been filtered out.
        if (resourceList.find(*rli))
            strList.append((*rli)->getName());
    }
    todo->setResources(strList);

    return todo;
}

bool
ICalReport::generate()
{
#if KDE_IS_VERSION(3,4,89)
    KCal::CalendarLocal cal("UTC");
#else
    KCal::CalendarLocal cal;
#endif

    if( !open())
    {
        qWarning(i18n("Can not open ICal File '%1' for writing!")
                 .arg(fileName));
        return FALSE;
    }

    TaskList filteredList;
    if (!filterTaskList(filteredList, 0, getHideTask(), getRollUpTask()))
        return FALSE;

    // Make sure that parents are in front of childs. We need this later to set
    // the relation.
    filteredList.setSorting(CoreAttributesList::TreeMode, 0);
    filteredList.setSorting(CoreAttributesList::StartUp, 1);
    sortTaskList(filteredList);

    ResourceList filteredResourceList;
    if (!filterResourceList(filteredResourceList, 0, hideResource,
                            rollUpResource))
        return FALSE;
    sortResourceList(filteredResourceList);

    QPtrDict<KCal::Todo> dict;
    for (TaskListIterator tli(filteredList); *tli != 0; ++tli)
    {
        // Generate a TODO item for each task.
        KCal::Todo* todo = generateTODO(*tli, filteredResourceList);

        // In case we have the parent in the list set the relation pointer.
        if((*tli)->getParent() && dict.find((*tli)->getParent()))
            todo->setRelatedTo(dict[(*tli)->getParent()]);

        // Insert the just created TODO into the calendar.
        cal.addTodo(todo);

        // Insert the TODO into the dict. We might need it as a parent.
        dict.insert(*tli, todo);
    }

    // Dump the calendar in ICal format into a text file.
    KCal::ICalFormat *format = new KCal::ICalFormat();
    s << format->toString(&cal) << endl;
    f.close();

    return TRUE;
}

#endif
