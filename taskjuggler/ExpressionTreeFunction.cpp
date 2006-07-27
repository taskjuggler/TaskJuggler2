/*
 * ExpressionTreeFunction.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "Interval.h"
#include "ExpressionTreeFunction.h"
#include "ExpressionTree.h"
#include "Operation.h"
#include "CoreAttributes.h"
#include "Project.h"
#include "Task.h"
#include "Resource.h"
#include "Account.h"
#include "Shift.h"
#include "debug.h"
#include "tjlib-internal.h"

#define KW(a) (a)

bool
ExpressionTreeFunction::checkCoreAttributesType(ExpressionTree* et)
{
    if (supportedCoreAttributes.isEmpty())
        return TRUE;

    QValueList<CAType>::iterator it;
    for (it = supportedCoreAttributes.begin(); it !=
         supportedCoreAttributes.end(); ++it)
       if (et->getCoreAttributes()->getType() == *it)
           return TRUE;

    return FALSE;
}

long
ExpressionTreeFunction::longCall(ExpressionTree* et,
                                 Operation* const ops[]) const
{
    if (DEBUGEX(15))
        qDebug("Resolving %s as long", name.latin1());
    return ((*this).*(longFunc))(et, ops);
}

const CoreAttributes*
ExpressionTreeFunction::findCoreAttributes(const CoreAttributes* ca,
                                           const QString& id) const
{
    const CoreAttributes* p;

    switch (ca->getType())
    {
        case CA_Task:
            p = ca->getProject()->getTask(id);
            break;
        case CA_Resource:
            p = ca->getProject()->getResource(id);
            break;
        case CA_Account:
            p = ca->getProject()->getAccount(id);
            break;
        case CA_Shift:
            p = ca->getProject()->getShift(id);
            break;
        default:
            p = 0;
    }

    return p;
}

long
ExpressionTreeFunction::hasAssignments(ExpressionTree* et,
                                       Operation* const ops[]) const
{
    /* Arguments:
       0 : scenario ID
       1 : interval start
       2 : interval end */
    if (et->getCoreAttributes()->getType() != CA_Task &&
        et->getCoreAttributes()->getType() != CA_Resource)
    {
        et->errorMessage(i18n("hasAssignments: '%1' is not a task or resource")
                         .arg(et->getCoreAttributes()->getFullId()));
        return 0;
    }

    int scenarioId = et->getCoreAttributes()->getProject()->
        getScenarioIndex(ops[0]->evalAsString(et)) - 1;
    if (scenarioId < 0)
    {
        et->errorMessage(i18n("hasAssignments: unknown scenario '%1'")
                         .arg(ops[0]->evalAsString(et)));
        return 0;
    }

    time_t start = ops[1]->evalAsTime(et);
    time_t end = ops[2]->evalAsTime(et);
    if (start > end)
    {
        et->errorMessage(i18n("hasAssignments: start date is larger than end "
                              "date"));
        return 0;
    }
    if (et->getCoreAttributes()->getProject()->getStart() > start)
        start = et->getCoreAttributes()->getProject()->getStart();
    if (et->getCoreAttributes()->getProject()->getEnd() < end)
        end = et->getCoreAttributes()->getProject()->getEnd();

    if (et->getCoreAttributes()->getType() == CA_Task)
        return ((Task*) et->getCoreAttributes())->getLoad
            (scenarioId, Interval(start, end), 0) > 0.0;
    else
        return ((Resource*) et->getCoreAttributes())->getLoad
            (scenarioId, Interval(start, end)) > 0.0;
}

long
ExpressionTreeFunction::isChildOf(ExpressionTree* et,
                                  Operation* const ops[]) const
{
    const CoreAttributes* p;
    if ((p = findCoreAttributes(et->getCoreAttributes(),
                                ops[0]->evalAsString(et))) == 0)
    {
        et->errorMessage(i18n("isChildOf: '%1' is unknown and not a "
                              "child of '%2'")
                         .arg(et->getCoreAttributes()->getFullId())
                         .arg(ops[0]->evalAsString(et)));
        return 0;
    }

    if (et->getCoreAttributes()->getType() != p->getType())
    {
        et->errorMessage(i18n
                         ("isChildOf: '%1' and '%2' must be of same type")
                         .arg(et->getCoreAttributes()->getFullId())
                         .arg(ops[0]->evalAsString(et)));
        return 0;
    }

    return et->getCoreAttributes()->isDescendentOf(p);
}

long
ExpressionTreeFunction::isParentOf(ExpressionTree* et,
                                   Operation* const ops[]) const
{
    const CoreAttributes* p;
    if ((p = findCoreAttributes(et->getCoreAttributes(),
                                ops[0]->evalAsString(et))) == 0)
    {
        et->errorMessage(i18n("isParentOf: '%1' is unknown and not a "
                              "child of '%2'")
                         .arg(et->getCoreAttributes()->getFullId())
                         .arg(ops[0]->evalAsString(et)));
        return 0;
    }

    if (et->getCoreAttributes()->getType() != p->getType())
    {
        et->errorMessage(i18n
                         ("isParentOf: '%1' and '%2' must be of same type")
                         .arg(et->getCoreAttributes()->getFullId())
                         .arg(ops[0]->evalAsString(et)));
        return 0;
    }

    return et->getCoreAttributes()->isParentOf(p);
}

long
ExpressionTreeFunction::isLeaf(ExpressionTree* et,
                               Operation* const []) const
{
    if (DEBUGEX(15))
    {
        qDebug("isLeaf() called for (%d) %s => %d",
               et->getCoreAttributes()->getType(),
               et->getCoreAttributes()->getId().latin1(),
               et->getCoreAttributes()->isLeaf());
    }
    return et->getCoreAttributes()->isLeaf();
}

long
ExpressionTreeFunction::treeLevel(ExpressionTree* et,
                                  Operation* const []) const
{
    return et->getCoreAttributes()->treeLevel() + 1;
}

long
ExpressionTreeFunction::isTask(ExpressionTree* et,
                               Operation* const ops[]) const
{
    if (!ops[0]->isValid())
    {
        if (!et->getCoreAttributes()->getProject()->
            getTask(ops[0]->evalAsString(et)))
        {
            et->errorMessage(i18n("isTask: task '%1' is unknown").
                arg(ops[0]->evalAsString(et)));
            return 0;
        }
        ops[0]->setValid();
    }

    return et->getCoreAttributes()->getType() == CA_Task &&
        et->getCoreAttributes()->getId() == ops[0]->evalAsString(et);
}

long
ExpressionTreeFunction::isATask(ExpressionTree* et,
                                Operation* const []) const
{
    return et->getCoreAttributes()->getType() == CA_Task;
}

long
ExpressionTreeFunction::isMilestone(ExpressionTree* et,
                                    Operation* const[]) const
{
    if (et->getCoreAttributes()->getType() != CA_Task)
        return 0;

    return ((Task*) et->getCoreAttributes())->isMilestone();
}

long
ExpressionTreeFunction::isResource(ExpressionTree* et,
                                   Operation* const ops[]) const
{
    if (!ops[0]->isValid())
    {
        if (!et->getCoreAttributes()->getProject()->
            getResource(ops[0]->evalAsString(et)))
        {
            et->errorMessage(i18n("isResource: resource '%1' is unknown").
                arg(ops[0]->evalAsString(et)));
            return 0;
        }
        ops[0]->setValid(TRUE);
    }

    return et->getCoreAttributes()->getType() == CA_Resource &&
        et->getCoreAttributes()->getId() == ops[0]->evalAsString(et);
}

long
ExpressionTreeFunction::isAResource(ExpressionTree* et,
                                    Operation* const []) const
{
    return et->getCoreAttributes()->getType() == CA_Resource;
}

long
ExpressionTreeFunction::isAccount(ExpressionTree* et,
                                  Operation* const ops[]) const
{
    if (!ops[0]->isValid())
    {
        if (!et->getCoreAttributes()->getProject()->
            getAccount(ops[0]->evalAsString(et)))
        {
            et->errorMessage(i18n("isAccount: account '%1' is unknown").
                arg(ops[0]->evalAsString(et)));
            return 0;
        }
        ops[0]->setValid(TRUE);
    }

    return et->getCoreAttributes()->getType() == CA_Account &&
        et->getCoreAttributes()->getId() == ops[0]->evalAsString(et);
}

long
ExpressionTreeFunction::isAnAccount(ExpressionTree* et,
                                    Operation* const []) const
{
    return et->getCoreAttributes()->getType() == CA_Account;
}

long
ExpressionTreeFunction::isTaskStatus(ExpressionTree* et,
                                     Operation* const ops[]) const
{
    if (et->getCoreAttributes()->getType() != CA_Task)
        return 0;

    int scenario;
    if ((scenario = et->getCoreAttributes()->
         getProject()->getScenarioIndex(ops[0]->evalAsString(et)) - 1) < 0)
    {
        et->errorMessage(i18n("isTaskStatus: Unknown scenario '%1")
                         .arg(ops[0]->evalAsString(et)));
        return 0;
    }

    static const char* stati[] = {
        KW("undefined"), KW("notstarted"), KW("inprogresslate"),
        KW("inprogress"), KW("ontime"), KW("inprogressearly"),
        KW("finished"), KW("late")
    };
    if (!ops[1]->isValid())
    {
        bool ok = FALSE;
        for (uint i = 0; i < (sizeof(stati) / sizeof(char*)); i++)
            if (ops[1]->evalAsString(et) == stati[i])
            {
                ok = TRUE;
                break;
            }
        if (!ok)
        {
            et->errorMessage(i18n("isTaskStatus: Unknown task status '%1'")
                             .arg(ops[1]->evalAsString(et)));
            return 0;
        }
        ops[1]->setValid();
    }

    return strcmp(stati[((Task*) et->getCoreAttributes())
                  ->getStatus(scenario)], ops[1]->evalAsString(et)) == 0;
}

long
ExpressionTreeFunction::startsBefore(ExpressionTree* et,
                                     Operation* const ops[]) const
{
    if (et->getCoreAttributes()->getType() != CA_Task)
        return 0;

    int scenario;
    if ((scenario = et->getCoreAttributes()->
         getProject()->getScenarioIndex(ops[0]->evalAsString(et)) - 1) < 0)
    {
        et->errorMessage(i18n("startsBefore: Unknown scenario '%1'")
                         .arg(ops[0]->evalAsString(et).latin1()));
        return 0;
    }

    return ((Task*) et->getCoreAttributes())->getStart(scenario) <
        ops[1]->evalAsTime(et);
}

long
ExpressionTreeFunction::startsAfter(ExpressionTree* et,
                                     Operation* const ops[]) const
{
    if (et->getCoreAttributes()->getType() != CA_Task)
        return 0;

    int scenario;
    if ((scenario = et->getCoreAttributes()->
         getProject()->getScenarioIndex(ops[0]->evalAsString(et)) - 1) < 0)
    {
        et->errorMessage(i18n("startsAfter: Unknown scenario '%1'")
                         .arg(ops[0]->evalAsString(et)));
        return 0;
    }

    return ((Task*) et->getCoreAttributes())->getStart(scenario) >=
        ops[1]->evalAsTime(et);
}

long
ExpressionTreeFunction::endsBefore(ExpressionTree* et,
                                     Operation* const ops[]) const
{
    if (et->getCoreAttributes()->getType() != CA_Task)
        return 0;

    int scenario;
    if ((scenario = et->getCoreAttributes()->
         getProject()->getScenarioIndex(ops[0]->evalAsString(et)) - 1) < 0)
    {
        et->errorMessage(i18n("endsBefore: Unknown scenario '%s'")
                         .arg(ops[0]->evalAsString(et)));
        return 0;
    }

    return ((Task*) et->getCoreAttributes())->getEnd(scenario) <
        ops[1]->evalAsTime(et);
}

long
ExpressionTreeFunction::endsAfter(ExpressionTree* et,
                                     Operation* const ops[]) const
{
    if (et->getCoreAttributes()->getType() != CA_Task)
        return 0;

    int scenario;
    if ((scenario = et->getCoreAttributes()->
         getProject()->getScenarioIndex(ops[0]->evalAsString(et)) - 1) < 0)
    {
        et->errorMessage(i18n("endsAfter: Unknown scenario '%1'")
                         .arg(ops[0]->evalAsString(et)));
        return 0;
    }

    return ((Task*) et->getCoreAttributes())->getEnd(scenario) >
        ops[1]->evalAsTime(et);
}

long
ExpressionTreeFunction::isSubTaskOf(ExpressionTree* et,
                                    Operation* const ops[]) const
{
    if (et->getCoreAttributes()->getType() != CA_Task)
        return 0;

    Task* p;
    if ((p = et->getCoreAttributes()->getProject()->getTask
         (ops[0]->evalAsString(et))) == 0)
    {
        et->errorMessage(i18n("isSubTaskOf: task '%1' is unknown")
                         .arg(ops[0]->evalAsString(et)));
        return 0;
    }

    return p->isSubTask((Task*) et->getCoreAttributes());
}

long
ExpressionTreeFunction::containsTask(ExpressionTree* et,
                                     Operation* const ops[]) const
{
    if (et->getCoreAttributes()->getType() != CA_Task)
        return 0;

    Task* st;
    if ((st = et->getCoreAttributes()->getProject()->getTask
         (ops[0]->evalAsString(et))) == 0)
    {
        et->errorMessage(i18n("containsTask: task '%1' is unknown")
                         .arg(et->getCoreAttributes()->getFullId()));
        return 0;
    }

    return ((Task*) et->getCoreAttributes())->isSubTask(st);
}

long
ExpressionTreeFunction::isTaskOfProject(ExpressionTree* et,
                                        Operation* const ops[]) const
{
    if (et->getCoreAttributes()->getType() != CA_Task)
        return 0;

    if (!ops[0]->isValid())
    {
        if (!et->getCoreAttributes()->getProject()->isValidId
            (ops[0]->evalAsString(et)))
        {
            et->errorMessage(i18n("isTaskOfProject: project ID '%1' is unkown")
                .arg(ops[0]->evalAsString(et)));
            return 0;
        }
        ops[0]->setValid();
    }

    return ops[0]->evalAsString(et) ==
        ((Task*) (et->getCoreAttributes()))->getProjectId();
}

long
ExpressionTreeFunction::isAllocated(ExpressionTree* et,
                                    Operation* const ops[]) const
{
    /* Arguments:
       0 : scenario id
       1 : interval start
       2 : interval end */
    if (et->getCoreAttributes()->getType() != CA_Resource)
    {
        et->errorMessage(i18n("isAllocated: '%1' is not a resource")
                         .arg(et->getCoreAttributes()->getFullId()));
        return 0;
    }

    int scenarioId = et->getCoreAttributes()->getProject()->
        getScenarioIndex(ops[0]->evalAsString(et)) - 1;
    if (scenarioId < 0)
    {
        et->errorMessage(i18n("isAllocated: unknown scenario '%1'")
                         .arg(ops[0]->evalAsString(et)));
        return 0;
    }

    time_t start = ops[1]->evalAsTime(et);
    time_t end = ops[2]->evalAsTime(et);
    if (start > end)
    {
        et->errorMessage(i18n("isAllocated: start date is larger than end "
                              "date"));
        return 0;
    }
    if (et->getCoreAttributes()->getProject()->getStart() > start)
        start = et->getCoreAttributes()->getProject()->getStart();
    if (et->getCoreAttributes()->getProject()->getEnd() < end)
        end = et->getCoreAttributes()->getProject()->getEnd();

    return ((Resource*) et->getCoreAttributes())->isAllocated
        (scenarioId, Interval(start, end),
         QString::null);
}

long
ExpressionTreeFunction::isDutyOf(ExpressionTree* et,
                                 Operation* const ops[]) const
{
    /* Arguments:
       0 : resource id
       1 : scenario id */
    if (et->getCoreAttributes()->getType() != CA_Task)
        return 0;

    Resource* resource = et->getCoreAttributes()->getProject()->
        getResource(ops[0]->evalAsString(et));
    if (!resource)
    {
        et->errorMessage(i18n("isDutyOf: resource '%1' is unknown")
                         .arg(ops[0]->evalAsString(et)));
        return 0;
    }

    int scenarioId = et->getCoreAttributes()->getProject()->
        getScenarioIndex(ops[1]->evalAsString(et)) - 1;
    if (scenarioId < 0)
    {
        et->errorMessage(i18n("isDutyOf: unknown scenario '%1'")
                         .arg(ops[1]->evalAsString(et)));
        return 0;
    }

    return ((Task*) et->getCoreAttributes())->isDutyOf(scenarioId, resource);
}

long
ExpressionTreeFunction::isAllocatedToProject(ExpressionTree* et,
                                             Operation* const ops[]) const
{
    /* Arguments:
       0 : project id
       1 : scenario id
       2 : interval start
       3 : interval end */
    if (et->getCoreAttributes()->getType() != CA_Resource)
        return 0;

    if (!ops[0]->isValid())
    {
        if (!et->getCoreAttributes()->getProject()->isValidId
            (ops[0]->evalAsString(et)))
        {
            et->errorMessage(i18n("isAllocatedToProject: project ID '%1'"
                                  "is unknown")
                             .arg(ops[0]->evalAsString(et)));
            return 0;
        }
        ops[0]->setValid();
    }

    int scenarioId = et->getCoreAttributes()->getProject()->
        getScenarioIndex(ops[1]->evalAsString(et)) - 1;
    if (scenarioId < 0)
    {
        et->errorMessage(i18n("isAllocatedToProject: unknown scenario '%1'")
                         .arg(ops[1]->evalAsString(et)));
        return 0;
    }
    time_t start = ops[2]->evalAsTime(et);
    time_t end = ops[3]->evalAsTime(et);
    if (start > end)
    {
        et->errorMessage(i18n("isAllocatedToProject: start date is larger "
                              "than end date"));
        return 0;
    }
    if (et->getCoreAttributes()->getProject()->getStart() > start)
        start = et->getCoreAttributes()->getProject()->getStart();
    if (et->getCoreAttributes()->getProject()->getEnd() < end)
        end = et->getCoreAttributes()->getProject()->getEnd();

    return ((Resource*) et->getCoreAttributes())->isAllocated
        (scenarioId, Interval(start, end),
         ops[0]->evalAsString(et));
}

long
ExpressionTreeFunction::isOnCriticalPath(ExpressionTree* et,
                                         Operation* const ops[]) const
{
    /* Arguments:
       0 : scenario id */
    if (et->getCoreAttributes()->getType() != CA_Task)
        return 0;

    int scenarioId = et->getCoreAttributes()->getProject()->
        getScenarioIndex(ops[0]->evalAsString(et)) - 1;
    if (scenarioId < 0)
    {
        et->errorMessage(i18n("isOnCriticalPath: unknown scenario '%1'")
                         .arg(ops[0]->evalAsString(et)));
        return 0;
    }

    Task* task = ((Task*) et->getCoreAttributes());
    return task->isOnCriticalPath(scenarioId);
}

long
ExpressionTreeFunction::isPlanAllocated(ExpressionTree* et,
                                        Operation* const ops[]) const
{
    if (et->getCoreAttributes()->getType() != CA_Resource)
    {
        et->errorMessage(i18n("isplanallocated: called for "
                              "non-resource '%1'")
               .arg(et->getCoreAttributes()->getFullId()));
        return 0;
    }
    int scenarioId = et->getCoreAttributes()->getProject()->
        getScenarioIndex("plan") - 1;
    if (scenarioId < 0)
    {
        et->errorMessage(i18n("isplanallocated: there is no "
                              "'plan' scenario."));
        return 0;
    }
    time_t start = ops[1]->evalAsTime(et);
    time_t end = ops[2]->evalAsTime(et);
    if (et->getCoreAttributes()->getProject()->getStart() > start)
        start = et->getCoreAttributes()->getProject()->getStart();
    if (et->getCoreAttributes()->getProject()->getEnd() < end)
        end = et->getCoreAttributes()->getProject()->getEnd();
    if (start > end)
    {
        et->errorMessage(i18n("isPlanAllocated: start date "
                              "is larger than end date"));
        return 0;
    }

    return ((Resource*) et->getCoreAttributes())->isAllocated
        (scenarioId, Interval(start, end),
         ops[0]->evalAsString(et));
}

long
ExpressionTreeFunction::isActualAllocated(ExpressionTree* et,
                                          Operation* const ops[]) const
{
    if (et->getCoreAttributes()->getType() != CA_Resource)
    {
        et->errorMessage(i18n("isactualallocated: called for non-resource"));
        return 0;
    }
    int scenarioId = et->getCoreAttributes()->getProject()->
        getScenarioIndex("actual") - 1;
    if (scenarioId < 0)
    {
        et->errorMessage(i18n("isactualallocated: there is no 'actual'"
                             " scenario."));
        return 0;
    }
    time_t start = ops[1]->evalAsTime(et);
    time_t end = ops[2]->evalAsTime(et);
    if (et->getCoreAttributes()->getProject()->getStart() > start)
        start = et->getCoreAttributes()->getProject()->getStart();
    if (et->getCoreAttributes()->getProject()->getEnd() < end)
        end = et->getCoreAttributes()->getProject()->getEnd();
    if (start > end)
    {
        et->errorMessage(i18n("isActualAllocated: start date "
                              "is larger than end date"));
        return 0;
    }

    return ((Resource*) et->getCoreAttributes())->isAllocated
        (scenarioId, Interval(start, end),
         ops[0]->evalAsString(et));
}

