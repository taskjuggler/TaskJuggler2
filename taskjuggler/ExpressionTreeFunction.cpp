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
#include "debug.h"

#define KW(a) (a)

bool
ExpressionTreeFunction::checkCoreAttributesType(ExpressionTree* et)
{
    if (supportedCoreAttributes.isEmpty())
        return TRUE;

    QStringList::iterator it;
    for (it = supportedCoreAttributes.begin(); it !=
         supportedCoreAttributes.end(); ++it)
       if (strcmp(et->getCoreAttributes()->getType(), *it) == 0)
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

long
ExpressionTreeFunction::isTask(const ExpressionTree* et, 
                               Operation* const ops[]) const
{
    if (DEBUGEX(15))
    {
        qDebug("isTask(%s) called for %s %s",
               ops[0]->debugString().latin1(),
               et->getCoreAttributes()->getType(),
               et->getCoreAttributes()->getId().latin1());
    }
    return strcmp(et->getCoreAttributes()->getType(), "Task") == 0 &&
        et->getCoreAttributes()->getId() == ops[0]->evalAsString(et);
}

long
ExpressionTreeFunction::isSubTaskOf(const ExpressionTree* et,
                                    Operation* const ops[])
const
{
    if (strcmp(et->getCoreAttributes()->getType(), "Task") != 0)
        return 0;
    Task* p;
    if ((p = et->getCoreAttributes()->getProject()->getTask
         (ops[0]->evalAsString(et))) == 0)
        return 0;
    return p->isSubTask((Task*) et->getCoreAttributes());
}

long
ExpressionTreeFunction::containsTask(const ExpressionTree* et,
                                     Operation* const ops[])
const
{
    if (strcmp(et->getCoreAttributes()->getType(), "Task") != 0)
        return 0;
    Task* st;
    if ((st = et->getCoreAttributes()->getProject()->getTask
         (ops[0]->evalAsString(et))) == 0)
        return 0;
    return ((Task*) et->getCoreAttributes())->isSubTask(st);
}

long
ExpressionTreeFunction::isMilestone(const ExpressionTree* et,
                                    Operation* const[])
const
{
    if (strcmp(et->getCoreAttributes()->getType(), "Task") != 0)
        return 0;
    return ((Task*) et->getCoreAttributes())->isMilestone();
}

long
ExpressionTreeFunction::isResource(const ExpressionTree* et,
                                   Operation* const ops[])
const
{
    return strcmp(et->getCoreAttributes()->getType(), "Resource") == 0 &&
        et->getCoreAttributes()->getId() == ops[0]->evalAsString(et);
}

long
ExpressionTreeFunction::isAccount(const ExpressionTree* et,
                                  Operation* const ops[]) const
{
    return strcmp(et->getCoreAttributes()->getType(), "Account") == 0 &&
        et->getCoreAttributes()->getId() == ops[0]->evalAsString(et);
}

long
ExpressionTreeFunction::isTaskStatus(const ExpressionTree* et,
                                     Operation* const ops[]) const
{
    if (strcmp(et->getCoreAttributes()->getType(), "Task") != 0)
        return 0;
    int scenario;
    if ((scenario = et->getCoreAttributes()->
         getProject()->getScenarioIndex(ops[0]->evalAsString(et)) - 1) < 0)
        qFatal("Unknown scenario %s",
               ops[0]->evalAsString(et).latin1());
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
    if (strcmp(et->getCoreAttributes()->getType(), "Task") != 0)
        return 0;
    int scenario;
    if ((scenario = et->getCoreAttributes()->
         getProject()->getScenarioIndex(ops[0]->evalAsString(et)) - 1) < 0)
        qFatal("Unknown scenario %s",
               ops[0]->evalAsString(et).latin1());

    return ((Task*) et->getCoreAttributes())->getStart(scenario) <
        ops[1]->evalAsTime(et);
}

long
ExpressionTreeFunction::startsAfter(const ExpressionTree* et,
                                     Operation* const ops[]) const
{
    if (strcmp(et->getCoreAttributes()->getType(), "Task") != 0)
        return 0;
    int scenario;
    if ((scenario = et->getCoreAttributes()->
         getProject()->getScenarioIndex(ops[0]->evalAsString(et)) - 1) < 0)
        qFatal("Unknown scenario %s",
               ops[0]->evalAsString(et).latin1());

    return ((Task*) et->getCoreAttributes())->getStart(scenario) >=
        ops[1]->evalAsTime(et);
}

long
ExpressionTreeFunction::endsBefore(const ExpressionTree* et,
                                     Operation* const ops[]) const
{
    if (strcmp(et->getCoreAttributes()->getType(), "Task") != 0)
        return 0;
    int scenario;
    if ((scenario = et->getCoreAttributes()->
         getProject()->getScenarioIndex(ops[0]->evalAsString(et)) - 1) < 0)
        qFatal("Unknown scenario %s",
               ops[0]->evalAsString(et).latin1());
    return ((Task*) et->getCoreAttributes())->getEnd(scenario) <
        ops[1]->evalAsTime(et);
}

long
ExpressionTreeFunction::endsAfter(const ExpressionTree* et,
                                     Operation* const ops[]) const
{
    if (strcmp(et->getCoreAttributes()->getType(), "Task") != 0)
        return 0;
    int scenario;
    if ((scenario = et->getCoreAttributes()->
         getProject()->getScenarioIndex(ops[0]->evalAsString(et)) - 1) < 0)
        qFatal("Unknown scenario %s",
               ops[0]->evalAsString(et).latin1());

    return ((Task*) et->getCoreAttributes())->getEnd(scenario) >
        ops[1]->evalAsTime(et);
}

long
ExpressionTreeFunction::isPlanAllocated(const ExpressionTree* et,
                                        Operation* const ops[]) const
{
    if (strcmp(et->getCoreAttributes()->getType(), "Resource") != 0)
        qFatal("Operation::evalFunction: isplanallocated called for "
               "non-resource");
    return ((Resource*) et->getCoreAttributes())->isAllocated
        (Task::Plan, Interval(ops[1]->evalAsTime(et), 
                              ops[2]->evalAsTime(et)), 
         ops[0]->evalAsString(et));
}

long
ExpressionTreeFunction::isActualAllocated(const ExpressionTree* et,
                                          Operation* const ops[]) const
{
    if (strcmp(et->getCoreAttributes()->getType(), "Resource") != 0)
        qFatal("Operation::evalFunction: isactualallocated called for "
               "non-resource");
    return ((Resource*) et->getCoreAttributes())->isAllocated
        (Task::Actual, Interval(ops[1]->evalAsTime(et), 
                                ops[2]->evalAsTime(et)), 
         ops[0]->evalAsString(et));
}

