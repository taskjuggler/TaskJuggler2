/*
 * ExportReport.cpp - TaskJuggler
 *
 * Copyright (c) 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <config.h>

#include <qfile.h>
#include <qmap.h>

#include "taskjuggler.h"
#include "Project.h"
#include "Scenario.h"
#include "Shift.h"
#include "ShiftSelection.h"
#include "Task.h"
#include "Resource.h"
#include "Booking.h"
#include "BookingList.h"
#include "ExportReport.h"
#include "ExpressionTree.h"
#include "Operation.h"
#include "CustomAttributeDefinition.h"
#include "TextAttribute.h"
#include "ReferenceAttribute.h"

#define KW(a) a

static QMap<QString, int> TaskAttributeDict;
typedef enum TADs { TA_FLAGS = 0, TA_NOTE, TA_PRIORITY, TA_MINSTART,
    TA_MAXSTART, TA_MINEND, TA_MAXEND, TA_COMPLETE, TA_RESPONSIBLE,
    TA_DEPENDS };

ExportReport::ExportReport(Project* p, const QString& f,
                           const QString& df, int dl) :
    Report(p, f, df, dl)
{
    if (TaskAttributeDict.empty())
    {
        TaskAttributeDict[KW("complete")] = TA_COMPLETE;
        TaskAttributeDict[KW("depends")] = TA_DEPENDS;
        TaskAttributeDict[KW("flags")] = TA_FLAGS;
        TaskAttributeDict[KW("maxend")] = TA_MAXEND;
        TaskAttributeDict[KW("maxstart")] = TA_MAXSTART;
        TaskAttributeDict[KW("minend")] = TA_MINEND;
        TaskAttributeDict[KW("minstart")] = TA_MINSTART;
        TaskAttributeDict[KW("note")] = TA_NOTE;
        TaskAttributeDict[KW("priority")] = TA_PRIORITY;
        TaskAttributeDict[KW("responsible")] = TA_RESPONSIBLE;
    }
    // show all tasks
    hideTask = new ExpressionTree(new Operation(0));
    // hide all resources
    hideResource = new ExpressionTree(new Operation(1));

    taskSortCriteria[0] = CoreAttributesList::TreeMode;
    taskSortCriteria[1] = CoreAttributesList::StartUp;
    taskSortCriteria[2] = CoreAttributesList::EndUp;
    resourceSortCriteria[0] = CoreAttributesList::TreeMode;
    resourceSortCriteria[1] = CoreAttributesList::IdUp;

    // All export reports default to just showing the first scenario.
    scenarios.append(0);

    masterFile = FALSE;
}

bool
ExportReport::generate()
{
    if (!open())
        return FALSE;

    if (timeStamp)
        s << "/*" << endl
            << " * This file has been generated by TaskJuggler " << VERSION
            << endl
            << " * at " << time2ISO(time(0)) << "." << endl
            << " */" << endl;
    s << "/*" << endl
       << " * For details about TaskJuggler see " << TJURL << endl
       << " */" << endl;

    TaskList filteredTaskList;
    if (!filterTaskList(filteredTaskList, 0, hideTask, rollUpTask))
        return FALSE;
    sortTaskList(filteredTaskList);

    ResourceList filteredResourceList;
    filterResourceList(filteredResourceList, 0, hideResource, rollUpResource);
    sortResourceList(filteredResourceList);

    if (masterFile)
    {
        if (!generateProjectProperty())
            return FALSE;
        if (!generateShiftList())
            return FALSE;
        if (!generateResourceList(filteredResourceList, filteredTaskList))
            return FALSE;
    }

    if (!generateProjectIds(filteredTaskList))
        return FALSE;

    if (!generateTaskList(filteredTaskList, filteredResourceList))
        return FALSE;
    if (!generateTaskAttributeList(filteredTaskList))
        return FALSE;

    if (!generateResourceAttributesList(filteredTaskList,
                                        filteredResourceList))
        return FALSE;

    f.close();
    return TRUE;
}

bool
ExportReport::generateProjectProperty()
{
    s << "project " << project->getId() << " \"" << project->getName()
        << "\" \"" << project->getVersion() << "\" "
        << time2tjp(getStart()) << " "
        << time2tjp(getEnd()) << " {" << endl;

    if (!generateCustomAttributeDeclaration
        ("task", project->getTaskAttributeDict()))
        return FALSE;
    if (!generateCustomAttributeDeclaration
        ("resource", project->getResourceAttributeDict()))
        return FALSE;
    if (!generateCustomAttributeDeclaration
        ("account", project->getAccountAttributeDict()))
        return FALSE;

    if (!project->getTimeZone().isEmpty())
        s << "  timezone \"" << project->getTimeZone() << "\"" << endl;
    s << "  dailyworkinghours " << project->getDailyWorkingHours() << endl;
    s << "  yearlyworkingdays " << project->getYearlyWorkingDays() << endl;
    s << "  timingresolution "
        << QString().sprintf("%ld", project->getScheduleGranularity() / 60)
        << "min" << endl;
    if (timeStamp)
        s << "  now " << time2tjp(project->getNow()) << endl;
    s << "  timeformat \"" << project->getTimeFormat() << "\"" << endl;
    s << "  shorttimeformat \"" << project->getShortTimeFormat() << "\""
        << endl;

    RealFormat rf = project->getCurrencyFormat();
    s << "  currencyformat \"" << rf.getSignPrefix() << "\" \""
        << rf.getSignSuffix() << "\" \""
        << rf.getThousandSep() << "\" \""
        << rf.getFractionSep() << "\" "
        << rf.getFracDigits() << endl;
    if (!project->getCurrency().isEmpty())
        s << "  currency " << project->getCurrency() << endl;
    if (project->getWeekStartsMonday())
        s << "  weekstartsmonday" << endl;
    else
        s << "  weekstartssunday" << endl;

    generateWorkingHours(project->getWorkingHours(), 0, 2);

    /* We need to make sure that the parent scenarios of all scenarios in the
     * scenario list are in the list as well. */
    QValueList<int> scL;
    QValueListIterator<int> sci;
    for (sci = scenarios.begin(); sci != scenarios.end(); ++sci)
        for (Scenario* scp = project->getScenario(*sci);
             scp != 0; scp = scp->getParent())
            if (scL.contains(scp->getIndex() - 1) == 0)
                scL.append(scp->getIndex() - 1);
    scenarios = scL;
    qHeapSort(scenarios);

    generateScenario(project->getScenario(0), 2);

    s << "}" << endl;

    return TRUE;
}

bool
ExportReport::generateCustomAttributeDeclaration(const QString& propertyName,
    QDictIterator<const CustomAttributeDefinition> it)
{
    if (!it.current())
        return TRUE;
    s << "  extend " << propertyName << " {" << endl;
    for ( ; it.current(); ++it)
    {
        s << "    ";
        switch (it.current()->getType())
        {
            case CAT_Text:
                s << "text";
                break;
            case CAT_Reference:
                s << "reference";
                break;
            default:
                qFatal("ExportReport::generateCustomAttributeDeclaration: "
                       "Unknown CAT %d", it.current()->getType());
                return FALSE;
        }
        s << " " << it.currentKey()
            << " \"" << it.current()->getName() << "\" " << endl;
    }
    s << "  }" << endl;

    return TRUE;
}

bool
ExportReport::generateScenario(const Scenario* scenario, int indent)
{
    /* Only include the scenario definition if it was in the list of requested
     * scenarios. */
    if (scenarios.contains(scenario->getIndex() - 1) == 0)
        return TRUE;

    s << QString().fill(' ', indent) << "scenario " << scenario->getId()
        << " \"" << scenario->getName() << "\" {" << endl;
    indent += 2;
    if (!scenario->getEnabled())
        s << QString().fill(' ', indent) << "disabled" << endl;
    if (scenario->getProjectionMode())
        s << QString().fill(' ', indent) << "projection" << endl;

    for (ScenarioListIterator sci(scenario->getSubListIterator());
         *sci != 0; ++sci)
        if (!generateScenario(*sci, indent))
            return FALSE;

    s << QString().fill(' ', indent - 2) << "}" << endl;

    return TRUE;
}

bool
ExportReport::generateShiftList()
{
    for (ShiftListIterator sli(project->getShiftListIterator());
         *sli != 0; ++sli)
    {
        if ((*sli)->getParent() == 0)
            if (!generateShift(*sli, 0))
                return FALSE;
    }

    return TRUE;
}

bool
ExportReport::generateShift(const Shift* shift, int indent)
{
    s << QString().fill(' ', indent)
        << "shift " << shift->getId() << " \"" << shift->getName()
        << "\" {" << endl;

    generateWorkingHours(shift->getWorkingHours(),
                         shift->getParent() ?
                         shift->getParent()->getWorkingHours() :
                         project->getWorkingHours(), indent + 2);

    for (ShiftListIterator sli(shift->getSubListIterator()); *sli; ++sli)
        if (!generateShift(*sli, indent + 2))
            return FALSE;

    s << QString().fill(' ', indent) << "}" << endl;

    return TRUE;
}

bool
ExportReport::generateWorkingHours(const QPtrList<const Interval>* const* wh,
                                   const QPtrList<const Interval>* const* ref,
                                   int indent)
{
    static const char* days[] =
    {
        "sun", "mon", "tue", "wed", "thu", "fri", "sat"
    };

    for (int i = 0; i < 7; ++i)
    {
        /* To reduce the size of the export file, we compare the working hours
         * to a reference set. This is the set that will be inherited during
         * file reading, so we don't have to list it twice. If a day matches,
         * it is not listed. */
        if (ref)
        {
            bool match = TRUE;
            QPtrListIterator<const Interval> whIt(*wh[i]);
            QPtrListIterator<const Interval> refIt(*ref[i]);
            if ((*whIt == 0 && *refIt != 0) ||
                (*whIt != 0 && *refIt == 0))
            {
                match = FALSE;
            }
            else
            {
                for ( ; *whIt; ++whIt, ++refIt)
                {
                    if (*refIt == 0 || **whIt != **refIt)
                    {
                        match = FALSE;
                        break;
                    }
                }
                if (*refIt != 0)
                    match = FALSE;
            }
            if (match)
                continue;
        }

        bool first = TRUE;
        s << QString().fill(' ', indent) <<
            "  workinghours " << days[i] << " ";
        QPtrListIterator<const Interval> it(*wh[i]);
        if (*it == 0)
        {
            s << "off";
        }
        else
        {
            for ( ; *it; ++it)
            {
                if (!first)
                    s << ", ";
                else
                    first = FALSE;

                s << QString().sprintf("%ld:%02ld",
                                       (*it)->getStart() / (60 * 60),
                                       ((*it)->getStart() % (60 * 60)) / 60)
                    << " - "
                    << QString().sprintf("%ld:%02ld",
                                       ((*it)->getEnd() + 1) / (60 * 60),
                                       (((*it)->getEnd() + 1) % (60 * 60))
                                       / 60);
            }
        }
        s << endl;
    }

    return TRUE;
}

bool
ExportReport::generateProjectIds(const TaskList& tasks)
{
    QStringList pids;

    for (TaskListIterator tli(tasks); *tli; ++tli)
        if (pids.contains((*tli)->getProjectId()) == 0)
            pids.append((*tli)->getProjectId());

    s << "projectids ";
    bool first = TRUE;
    for (QStringList::iterator it = pids.begin(); it != pids.end(); ++it)
    {
        if (first)
            first = FALSE;
        else
            s << ", ";
        s << *it;
    }
    s << endl;

    return TRUE;
}

bool
ExportReport::generateResourceList(ResourceList& filteredResourceList,
                                   TaskList&)
{
    for (ResourceListIterator rli(filteredResourceList); *rli != 0; ++rli)
        if ((*rli)->getParent() == 0)
            if (!generateResource(filteredResourceList, *rli, 0))
                return FALSE;

    return TRUE;
}

bool
ExportReport::generateResource(ResourceList& filteredResourceList,
                               const Resource* resource, int indent)
{
    s << QString().fill(' ', indent) << "resource " << resource->getId()
        << " \"" << resource->getName() << "\"" << " {" << endl;

    for (ResourceListIterator srli(resource->getSubListIterator());
         *srli != 0; ++srli)
    {
        if (filteredResourceList.findRef(*srli) >= 0)
        {
            if (!generateResource(filteredResourceList, *srli, indent + 2))
                return FALSE;
        }
    }

    generateWorkingHours(resource->getWorkingHours(),
                         resource->getParent() ?
                         resource->getParent()->getWorkingHours() :
                         project->getWorkingHours(), 2);

    for (ShiftSelectionListIterator sli(*resource->getShiftList()); *sli; ++sli)
    {
        s << "  shift " << (*sli)->getShift()->getId() << " "
            << time2tjp((*sli)->getPeriod().getStart()) << " - "
            << time2tjp((*sli)->getPeriod().getEnd() + 1) << endl;
    }

    s << QString().fill(' ', indent) << "}" << endl;

    return TRUE;
}

bool
ExportReport::generateTaskList(TaskList& filteredTaskList,
                               ResourceList&)
{
    for (TaskListIterator tli(filteredTaskList); *tli != 0; ++tli)
        if ((*tli)->getParent() == 0 ||
            (*tli)->getParent()->getId() + "." == taskRoot)
            if (!generateTask(filteredTaskList, *tli, 0))
                return FALSE;

    return TRUE;
}

bool
ExportReport::generateTask(TaskList& filteredTaskList, const Task* task,
                           int indent)
{
    QString taskId = task->getId();
    if (task->getParent())
        taskId = taskId.right(taskId.length() - 1 -
                              task->getParent()->getId().length());
    s << QString().fill(' ', indent) << "task " << taskId
        << " \"" << task->getName() << "\"" << " {" << endl;

    /* Generate inheritable attributes prior to the sub tasks, so we don't
     * have to have to generate them for the sub tasks as well. */
    if (task->getParent() == 0 ||
        task->getParent()->getId() + '.' == taskRoot ||
        task->getParent()->getProjectId() != task->getProjectId())
    {
        s << QString().fill(' ', indent + 2) << "projectid "
            << task->getProjectId() << endl;
    }

    for (QStringList::Iterator it = taskAttributes.begin();
         it != taskAttributes.end(); ++it)
    {
        if (!TaskAttributeDict.contains(*it))
            continue;
        switch (TaskAttributeDict[*it])
        {
            case TA_DEPENDS:
                generateDepList(filteredTaskList, task,
                                task->getDependsIterator(), "depends", indent);
                generateDepList(filteredTaskList, task,
                                task->getPrecedesIterator(), "precedes",
                                indent);
                break;
            default:
                break;
        }
    }

    /* If a container task has sub tasks that are exported as well, we do
     * not export start/end date for those container tasks. */
    bool taskHasNoSubTasks = TRUE;
    for (TaskListIterator stli(task->getSubListIterator());
         *stli != 0; ++stli)
    {
        if (filteredTaskList.findRef(*stli) >= 0)
        {
            taskHasNoSubTasks = FALSE;
            if (!generateTask(filteredTaskList, *stli, indent + 2))
                return FALSE;
        }
    }
    if (taskHasNoSubTasks)
    {
        for (QValueListIterator<int> it = scenarios.begin();
             it != scenarios.end(); ++it)
        {
            QString start = time2rfc(task->getStart(*it));
            QString end = time2rfc(task->getEnd(*it) + 1);
            s << QString().fill(' ', indent + 2) <<
                project->getScenarioId(*it) << ":"
                << "start " << start << endl;
            if (!task->isMilestone())
            {
                s << QString().fill(' ', indent + 2)
                    << project->getScenarioId(*it) << ":"
                    << "end " << end << endl;
            }
            if (task->getScheduled(*it))
                s << QString().fill(' ', indent + 2)
                    << project->getScenarioId(*it) << ":"
                    << "scheduled" << endl;
        }
    }

    if (task->isMilestone())
        s << QString().fill(' ', indent + 2) << "milestone " << endl;

    s << QString().fill(' ', indent + 2)
        << "scheduling " << (task->getScheduling() == Task::ASAP ?
                             "asap" : "alap") << endl;

    s << QString().fill(' ', indent) << "}" << endl;

    return TRUE;
}

bool
ExportReport::generateDepList(TaskList& filteredTaskList, const Task* task,
                              QPtrListIterator<TaskDependency> depIt,
                              const char* tag, int indent)
{
    bool first = TRUE;
    bool prev = (tag == "depends");
    for ( ; *depIt != 0; ++depIt)

    {
        /* For each of the Tasks in the depency list, we check that it's not
         * already in the parent's dependency list.
         *
         * Save current list item since findRef() modifies
         * it. Remember, we are still iterating the list. */
        CoreAttributes* curr = filteredTaskList.current();
        if (filteredTaskList.findRef((*depIt)->getTaskRef()) > -1 &&
            (task->getParent() == 0 ||
             !(prev ? task->getParent()->hasPrevious((*depIt)->getTaskRef()) :
               task->getParent()->hasFollower((*depIt)->getTaskRef()))))
        {
            if (first)
            {
                s << QString().fill(' ', indent + 2)
                    << tag << " ";
                first = FALSE;
            }
            else
                s << ", ";
            s << stripTaskRoot(((*depIt)->getTaskRef())->getId());

            bool showOptionals = FALSE;
            for (int sc = 0; sc < project->getMaxScenarios(); ++sc)
                if ((*depIt)->getGapDuration(sc) != 0 ||
                    (*depIt)->getGapLength(sc) != 0)
                {
                    showOptionals = TRUE;
                    break;
                }
            if (showOptionals)
            {
                s << " { ";
                for (int sc = 0; sc < project->getMaxScenarios(); ++sc)
                {
                    if ((*depIt)->getGapDuration(sc) != 0)
                        s << project->getScenarioId(sc)
                            << ":gapduration "
                            << (*depIt)->getGapDuration(sc) / 3600.0 << "h ";
                    if ((*depIt)->getGapLength(sc) != 0)
                        s << project->getScenarioId(sc)
                            << ":gaplength "
                            << (*depIt)->getGapLength(sc) / 3600.0 << "h ";
                }
                s << "}";
            }
        }
        /* Restore current list item to continue
         * iteration. */
        filteredTaskList.findRef(curr);
    }
    if (!first)
        s << endl;

    return TRUE;
}

bool
ExportReport::generateTaskAttributeList(TaskList& filteredTaskList)
{
    if (taskAttributes.isEmpty())
        return TRUE;

    if (taskAttributes.contains("flags"))
    {
        FlagList allFlags;
        for (TaskListIterator tli(filteredTaskList); *tli != 0; ++tli)
        {
            QStringList fl = (*tli)->getFlagList();
            for (QStringList::Iterator jt = fl.begin();
                 jt != fl.end(); ++jt)
            {
                if (allFlags.find(*jt) == allFlags.end())
                    allFlags.append(*jt);
            }

        }
        if (allFlags.begin() != allFlags.end())
        {
            s << "flags ";
            for (QStringList::Iterator it = allFlags.begin();
                 it != allFlags.end(); ++it)
            {
                if (it != allFlags.begin())
                    s << ", ";
                s << *it;
            }
            s << endl;
        }
    }

    for (TaskListIterator tli(filteredTaskList); *tli != 0; ++tli)
        if ((*tli)->getParent() == 0 ||
            (*tli)->getParent()->getId() + "." == taskRoot)
            if (!generateTaskSupplement(filteredTaskList, *tli, 0))
                return FALSE;

    return TRUE;
}

bool
ExportReport::generateTaskSupplement(TaskList& filteredTaskList,
                                     const Task* task, int indent)
{
    QString taskId = task->getId();
    if (task->getParent())
        taskId = taskId.right(taskId.length() - 1 -
                              task->getParent()->getId().length());
    s << QString().fill(' ', indent) << "supplement task " << taskId
        << " {" << endl;

    for (TaskListIterator stli(task->getSubListIterator());
         *stli != 0; ++stli)
    {
        if (filteredTaskList.findRef(*stli) >= 0)
        {
            if (!generateTaskSupplement(filteredTaskList, *stli, indent + 2))
                return FALSE;
        }
    }

    for (QStringList::Iterator it = taskAttributes.begin();
         it != taskAttributes.end(); ++it)
    {
        if (!TaskAttributeDict.contains(*it))
        {
            if (task->getCustomAttribute(*it))
                generateCustomAttributeValue(*it, task, indent);
            continue;
        }
        switch (TaskAttributeDict[*it])
        {
            case TA_FLAGS:
                {
                    if (task->getFlagList().empty())
                        break;

                    s << QString().fill(' ', indent + 2) << "flags ";
                    QStringList fl = task->getFlagList();
                    bool first = TRUE;
                    for (QStringList::Iterator jt = fl.begin();
                         jt != fl.end(); ++jt)
                    {
                        if (!first)
                            s << ", ";
                        else
                            first = FALSE;
                        s << *jt;
                    }
                    s << endl;
                    break;
                }
            case TA_NOTE:
                if (task->getNote() != "")
                {
                    s << QString().fill(' ', indent + 2)
                        << "note \"" << task->getNote() << "\"" << endl;
                }
                break;
            case TA_PRIORITY:
                if (task->getParent() == 0 ||
                    task->getParent()->getId() + '.' == taskRoot ||
                    task->getParent()->getPriority() != task->getPriority())
                {
                    s << QString().fill(' ', indent + 2)
                        << "priority "
                        << QString().sprintf("%d", task->getPriority()) << endl;
                }
                break;
            case TA_MINSTART:
                {
                    for (QValueListIterator<int> it = scenarios.begin();
                         it != scenarios.end(); ++it)
                    {
                        if (task->getMinStart(*it) != 0)
                        {
                            s << QString().fill(' ', indent + 2)
                                << project->getScenarioId(*it) << ":"
                                << "minstart "
                                << time2rfc(task->getMinStart(*it))
                                << endl;
                        }
                    }
                    break;
                }
            case TA_MAXSTART:
                {
                    for (QValueListIterator<int> it = scenarios.begin();
                         it != scenarios.end(); ++it)
                    {
                        if (task->getMaxStart(*it) != 0)
                        {
                            s << QString().fill(' ', indent + 2)
                                << project->getScenarioId(*it) << ":"
                                << "maxstart "
                                << time2rfc(task->getMaxStart(*it))
                                << endl;
                        }
                    }
                    break;
                }
            case TA_MINEND:
                {
                    for (QValueListIterator<int> it = scenarios.begin();
                         it != scenarios.end(); ++it)
                    {
                        if (task->getMinEnd(*it) != 0)
                        {
                            s << QString().fill(' ', indent + 2)
                                << project->getScenarioId(*it) << ":"
                                << "minend "
                                << time2rfc(task->getMinEnd(*it) + 1)
                                << endl;
                        }
                    }
                    break;
                }
            case TA_MAXEND:
                {
                    for (QValueListIterator<int> it = scenarios.begin();
                         it != scenarios.end(); ++it)
                    {
                        if (task->getMaxEnd(*it) != 0)
                        {
                            s << QString().fill(' ', indent + 2)
                                << project->getScenarioId(*it) << ":"
                                << "maxend "
                                << time2rfc(task->getMaxEnd(*it) + 1)
                                << endl;
                        }
                    }
                    break;
                }
            case TA_COMPLETE:
                {
                    for (QValueListIterator<int> it = scenarios.begin();
                         it != scenarios.end(); ++it)
                    {
                        if (task->getComplete(*it) >= 0.0)
                        {
                            s << QString().fill(' ', indent + 2)
                                << project->getScenarioId(*it) << ":"
                                << "complete "
                                << (int) task->getComplete(*it)
                                << endl;
                        }
                    }
                    break;
                }
            case TA_RESPONSIBLE:
                if (task->getResponsible())
                {
                    s << QString().fill(' ', indent + 2) << "responsible "
                        << task->getResponsible()->getId() << endl;
                }
                break;
            case TA_DEPENDS:
                // handled in generateTaskList
                break;
            default:
                qDebug("ExportReport::generateTaskSupplement(): "
                       "Unknown task attribute %s", (*it).latin1());
                return FALSE;
        }
    }

    s << QString().fill(' ', indent) << "}" << endl;

    return TRUE;
}

bool
ExportReport::generateResourceAttributesList(TaskList& filteredTaskList,
                                             ResourceList& filteredResourceList)
{
    for (ResourceListIterator rli(filteredResourceList); *rli != 0; ++rli)
    {
        bool first = TRUE;
        for (QValueListIterator<int> sit = scenarios.begin();
             sit != scenarios.end(); ++sit)
        {
            BookingList bl = (*rli)->getJobs(*sit);
            bl.setAutoDelete(TRUE);
            if (bl.isEmpty())
                continue;

            for (BookingListIterator bli(bl); *bli != 0; ++bli)
            {
                if (filteredTaskList.findRef((*bli)->getTask()) >= 0)
                {
                    if (first)
                    {
                        s << "supplement resource " << (*rli)->getId()
                            << " {" << endl;
                        first = FALSE;
                    }
                    QString start = time2rfc((*bli)->getStart());
                    QString end = time2rfc((*bli)->getEnd() + 1);
                    s << "  " << project->getScenarioId(*sit) << ":booking ";
                    s << start << " " << end
                        << " " << stripTaskRoot((*bli)->getTask()->getId())
                        << endl;
                }
            }
        }
        if (!first)
            s << "}" << endl;
    }

    return TRUE;
}

bool
ExportReport::generateCustomAttributeValue(const QString& id,
                                           const CoreAttributes* property,
                                           int indent)
{
    s << QString().fill(' ', indent + 2) << id << " ";
    const CustomAttribute* ca = property->getCustomAttribute(id);
    switch (ca->getType())
    {
        case CAT_Text:
            s << "\"" << ((const TextAttribute*) ca)->getText()
                << "\"" << endl;
            break;
        case CAT_Reference:
            {
                const ReferenceAttribute* a =
                    (const ReferenceAttribute*) ca;
                s << "\"" << a->getURL() << "\" { label \""
                    << a->getLabel() << "\" }" << endl;
                break;
            }
        default:
            qFatal("ExportReport::"
                   "generateTaskAttributeList: "
                   "Unknown CAT %d",
                   ca->getType());
    }

    return TRUE;
}


bool
ExportReport::addTaskAttribute(const QString& ta)
{
    if (ta == KW("all"))
    {
        QMap<QString, int>::ConstIterator it;
        for (it = TaskAttributeDict.begin(); it != TaskAttributeDict.end();
             ++it)
        {
            if (taskAttributes.findIndex(it.key()) < 0)
                taskAttributes.append(it.key());
        }
        for (QDictIterator<const CustomAttributeDefinition>
             it(project->getTaskAttributeDict()); *it; ++it)
            taskAttributes.append(it.currentKey());

        return TRUE;
    }

    /* Make sure the 'ta' is a valid attribute name and that we don't
     * insert it twice into the list. Trying to insert it twice it not an
     * error though. */
    if (TaskAttributeDict.find(ta) == TaskAttributeDict.end() &&
        project->getTaskAttribute(ta) == 0)
        return FALSE;

    if (taskAttributes.findIndex(ta) >= 0)
        return TRUE;
    taskAttributes.append(ta);
    return TRUE;
}

