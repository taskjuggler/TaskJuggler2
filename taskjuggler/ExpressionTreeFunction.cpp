/*
 * ExpressionTreeFunction.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
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
ExpressionTreeFunction::longCall(const ExpressionTree* et, 
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
    
    if (ca->getType() == CA_Task)
        p = ca->getProject()->getTask(id);
    else if (ca->getType() == CA_Resource)
        p = ca->getProject()->getResource(id);
    else if (ca->getType() == CA_Account)
        p = ca->getProject()->getAccount(id);
    else if (ca->getType() == CA_Shift)
        p = ca->getProject()->getShift(id);
    else
        return 0;

    return p;
}

long
ExpressionTreeFunction::isChildOf(const ExpressionTree* et,
                                  Operation* const ops[]) const
{
    const CoreAttributes* p;
    if ((p = findCoreAttributes(et->getCoreAttributes(),
                                ops[0]->evalAsString(et))) == 0)
    {
        qDebug("%s is no child of %s",
               et->getCoreAttributes()->getFullId().latin1(),
               ops[0]->evalAsString(et).latin1());
        return 0;
    }

    return et->getCoreAttributes()->isDescendentOf(p);
}

long
ExpressionTreeFunction::isParentOf(const ExpressionTree* et,
                                   Operation* const ops[]) const
{
    const CoreAttributes* p;
    if ((p = findCoreAttributes(et->getCoreAttributes(),
                                ops[0]->evalAsString(et))) == 0)
        return 0;

    return et->getCoreAttributes()->isParentOf(p);
}

long
ExpressionTreeFunction::isLeaf(const ExpressionTree* et, 
                               Operation* const []) const
{
    return et->getCoreAttributes()->isLeaf();
}

long
ExpressionTreeFunction::treeLevel(const ExpressionTree* et,
                                  Operation* const []) const
{
    return et->getCoreAttributes()->treeLevel() + 1;
}

long
ExpressionTreeFunction::isTask(const ExpressionTree* et, 
                               Operation* const ops[]) const
{
    if (DEBUGEX(15))
    {
        qDebug("isTask(%s) called for (%d) %s",
               ops[0]->debugString().latin1(),
               et->getCoreAttributes()->getType(),
               et->getCoreAttributes()->getId().latin1());
    }
    return et->getCoreAttributes()->getType() == CA_Task &&
        et->getCoreAttributes()->getId() == ops[0]->evalAsString(et);
}

long
ExpressionTreeFunction::isMilestone(const ExpressionTree* et,
                                    Operation* const[]) const
{
    if (et->getCoreAttributes()->getType() != CA_Task)
        return 0;
    return ((Task*) et->getCoreAttributes())->isMilestone();
}

long
ExpressionTreeFunction::isResource(const ExpressionTree* et,
                                   Operation* const ops[]) const
{
    return et->getCoreAttributes()->getType() == CA_Resource &&
        et->getCoreAttributes()->getId() == ops[0]->evalAsString(et);
}

long
ExpressionTreeFunction::isAccount(const ExpressionTree* et,
                                  Operation* const ops[]) const
{
    return et->getCoreAttributes()->getType() == CA_Account &&
        et->getCoreAttributes()->getId() == ops[0]->evalAsString(et);
}

long
ExpressionTreeFunction::isTaskStatus(const ExpressionTree* et,
                                     Operation* const ops[]) const
{
    if (et->getCoreAttributes()->getType() != CA_Task)
        return 0;
    int scenario;
    if ((scenario = et->getCoreAttributes()->
         getProject()->getScenarioIndex(ops[0]->evalAsString(et)) - 1) < 0)
        qFatal(i18n("ExpressionTreeFunction::isTaskStatus(): "
                    "Unknown scenario '%1'")
               .arg(ops[0]->evalAsString(et).latin1()));
    static const char* stati[] = {
        KW("undefined"), KW("notstarted"), KW("inprogresslate"),
        KW("inprogress"), KW("ontime"), KW("inprogressearly"),
        KW("finished")
    };
    return strcmp(stati[((Task*) et->getCoreAttributes())
                  ->getStatus(scenario)], ops[1]->evalAsString(et)) == 0;
}

long
ExpressionTreeFunction::startsBefore(const ExpressionTree* et,
                                     Operation* const ops[]) const
{
    if (et->getCoreAttributes()->getType() != CA_Task)
        return 0;
    int scenario;
    if ((scenario = et->getCoreAttributes()->
         getProject()->getScenarioIndex(ops[0]->evalAsString(et)) - 1) < 0)
        qFatal(i18n("ExpressionTreeFunction::startsBefore: "
                    "Unknown scenario '%1'")
               .arg(ops[0]->evalAsString(et).latin1()));

    return ((Task*) et->getCoreAttributes())->getStart(scenario) <
        ops[1]->evalAsTime(et);
}

long
ExpressionTreeFunction::startsAfter(const ExpressionTree* et,
                                     Operation* const ops[]) const
{
    if (et->getCoreAttributes()->getType() != CA_Task)
        return 0;
    int scenario;
    if ((scenario = et->getCoreAttributes()->
         getProject()->getScenarioIndex(ops[0]->evalAsString(et)) - 1) < 0)
        qFatal(i18n("ExpressionTreeFunction::startsAfter: "
                    "Unknown scenario '%1'")
               .arg(ops[0]->evalAsString(et).latin1()));

    return ((Task*) et->getCoreAttributes())->getStart(scenario) >=
        ops[1]->evalAsTime(et);
}

long
ExpressionTreeFunction::endsBefore(const ExpressionTree* et,
                                     Operation* const ops[]) const
{
    if (et->getCoreAttributes()->getType() != CA_Task)
        return 0;
    int scenario;
    if ((scenario = et->getCoreAttributes()->
         getProject()->getScenarioIndex(ops[0]->evalAsString(et)) - 1) < 0)
        qFatal(i18n("EpessionTreeFunction::endsBefore: Unknown scenario '%1'")
               .arg(ops[0]->evalAsString(et).latin1()));
    return ((Task*) et->getCoreAttributes())->getEnd(scenario) <
        ops[1]->evalAsTime(et);
}

long
ExpressionTreeFunction::endsAfter(const ExpressionTree* et,
                                     Operation* const ops[]) const
{
    if (et->getCoreAttributes()->getType() != CA_Task)
        return 0;
    int scenario;
    if ((scenario = et->getCoreAttributes()->
         getProject()->getScenarioIndex(ops[0]->evalAsString(et)) - 1) < 0)
        qFatal(i18n("ExpressionTreeFunction::endsAfter: Unknown scenario '%1'")
               .arg(ops[0]->evalAsString(et).latin1()));

    return ((Task*) et->getCoreAttributes())->getEnd(scenario) >
        ops[1]->evalAsTime(et);
}

long
ExpressionTreeFunction::isSubTaskOf(const ExpressionTree* et,
                                    Operation* const ops[]) const
{
    if (et->getCoreAttributes()->getType() != CA_Task)
        return 0;
    Task* p;
    if ((p = et->getCoreAttributes()->getProject()->getTask
         (ops[0]->evalAsString(et))) == 0)
        return 0;
    return p->isSubTask((Task*) et->getCoreAttributes());
}

long
ExpressionTreeFunction::containsTask(const ExpressionTree* et,
                                     Operation* const ops[]) const
{
    if (et->getCoreAttributes()->getType() != CA_Task)
        return 0;
    Task* st;
    if ((st = et->getCoreAttributes()->getProject()->getTask
         (ops[0]->evalAsString(et))) == 0)
        return 0;
    return ((Task*) et->getCoreAttributes())->isSubTask(st);
}

long
ExpressionTreeFunction::isTaskOfProject(const ExpressionTree* et,
                                        Operation* const ops[]) const
{
    if (et->getCoreAttributes()->getType() != CA_Task)
        return 0;
    return ops[0]->evalAsString(et) ==
        ((Task*) (et->getCoreAttributes()))->getProjectId();
}

long
ExpressionTreeFunction::isAllocated(const ExpressionTree* et,
                                    Operation* const ops[]) const
{
    /* Arguments:
       0 : scenario id
       1 : interval start
       2 : interval end */
    if (et->getCoreAttributes()->getType() != CA_Resource)
        qFatal(i18n("ExpressionTreeFunction::isAllocated() called for "
                    "non-resource"));
    int scenarioId = et->getCoreAttributes()->getProject()->
        getScenarioIndex(ops[0]->evalAsString(et)) - 1;
    if (scenarioId < 0)
        qFatal(i18n("ExpressionTreeFunction::isAllocated() called for "
                    "unknown '%1' scenario.")
               .arg(ops[0]->evalAsString(et)));
    time_t start = ops[1]->evalAsTime(et);
    time_t end = ops[2]->evalAsTime(et);
    if (et->getCoreAttributes()->getProject()->getStart() > start)
        start = et->getCoreAttributes()->getProject()->getStart();
    if (et->getCoreAttributes()->getProject()->getEnd() < end)
        end = et->getCoreAttributes()->getProject()->getEnd();
    if (start > end)
        qFatal("ExpressionTreeFunction::isAllocated: start date "
               "is larger than end date");

    return ((Resource*) et->getCoreAttributes())->isAllocated
        (scenarioId, Interval(start, end), 
         QString::null);
}

long
ExpressionTreeFunction::isAllocatedToProject(const ExpressionTree* et,
                                             Operation* const ops[]) const
{
    /* Arguments:
       0 : project id
       1 : scenario id
       2 : interval start
       3 : interval end */
    if (et->getCoreAttributes()->getType() != CA_Resource)
        qFatal(i18n("ExpressionTreeFunction::isAllocatedToProject() "
                    "called for non-resource"));
    int scenarioId = et->getCoreAttributes()->getProject()->
        getScenarioIndex(ops[1]->evalAsString(et)) - 1;
    if (scenarioId < 0)
        qFatal(i18n("ExpressionTreeFunction::isAllocatedToProject() "
                    "called for unknown '%1' scenario.")
               .arg(ops[1]->evalAsString(et)));
    time_t start = ops[2]->evalAsTime(et);
    time_t end = ops[3]->evalAsTime(et);
    if (et->getCoreAttributes()->getProject()->getStart() > start)
        start = et->getCoreAttributes()->getProject()->getStart();
    if (et->getCoreAttributes()->getProject()->getEnd() < end)
        end = et->getCoreAttributes()->getProject()->getEnd();
    if (start > end)
        qFatal("ExpressionTreeFunction::isAllocatedToProject(): start date "
               "is larger than end date");

    return ((Resource*) et->getCoreAttributes())->isAllocated
        (scenarioId, Interval(start, end), 
         ops[0]->evalAsString(et));
}

long
ExpressionTreeFunction::isPlanAllocated(const ExpressionTree* et,
                                        Operation* const ops[]) const
{
    if (et->getCoreAttributes()->getType() != CA_Resource)
        qFatal(i18n("ExpressionTreeFunction::isplanallocated() called for "
                    "non-resource '%1'")
               .arg(et->getCoreAttributes()->getId().latin1()));
    int scenarioId = et->getCoreAttributes()->getProject()->
        getScenarioIndex("plan") - 1;
    if (scenarioId < 0)
        qFatal(i18n("ExpressionTreeFunction::isplanallocated() called, but "
                    "there is no 'plan' scenario."));
    time_t start = ops[1]->evalAsTime(et);
    time_t end = ops[2]->evalAsTime(et);
    if (et->getCoreAttributes()->getProject()->getStart() > start)
        start = et->getCoreAttributes()->getProject()->getStart();
    if (et->getCoreAttributes()->getProject()->getEnd() < end)
        end = et->getCoreAttributes()->getProject()->getEnd();
    if (start > end)
        qFatal("ExpressionTreeFunction::isPlanAllocated(): start date "
               "is larger than end date");

    return ((Resource*) et->getCoreAttributes())->isAllocated
        (scenarioId, Interval(start, end), 
         ops[0]->evalAsString(et));
}

long
ExpressionTreeFunction::isActualAllocated(const ExpressionTree* et,
                                          Operation* const ops[]) const
{
    if (et->getCoreAttributes()->getType() != CA_Resource)
        qFatal("ExpressionTreeFunction::isactualallocated() called for "
               "non-resource");
    int scenarioId = et->getCoreAttributes()->getProject()->
        getScenarioIndex("actual") - 1;
    if (scenarioId < 0)
        qFatal("ExpressionTreeFunction::isactualallocated() called, but "
               "there is no 'actual' scenario.");
    time_t start = ops[1]->evalAsTime(et);
    time_t end = ops[2]->evalAsTime(et);
    if (et->getCoreAttributes()->getProject()->getStart() > start)
        start = et->getCoreAttributes()->getProject()->getStart();
    if (et->getCoreAttributes()->getProject()->getEnd() < end)
        end = et->getCoreAttributes()->getProject()->getEnd();
    if (start > end)
        qFatal("ExpressionTreeFunction::isAccountAllocated(): start date "
               "is larger than end date");

    return ((Resource*) et->getCoreAttributes())->isAllocated
        (scenarioId, Interval(start, end), 
         ops[0]->evalAsString(et));
}

