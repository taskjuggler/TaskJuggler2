/*
 * ExpressionTreeFunction.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef _ExpressionTreeFunction_h_
#define _ExpressionTreeFunction_h_

#include "qstring.h"
#include "qstringlist.h"

#include "taskjuggler.h"

class ExpressionTreeFunction;
class ExpressionTree;
class Operation;
class CoreAttributes;

typedef long (ExpressionTreeFunction::*ExpressionTreeFunctionLongPtr)
    (ExpressionTree*, Operation* const ops[]) const;

class ExpressionTreeFunction
{
public:
    ExpressionTreeFunction(const QString& n, ExpressionTreeFunctionLongPtr f,
                           int a) :
       name(n), longFunc(f), args(a) { }
    ~ExpressionTreeFunction() { }

    const QString& getName() const { return name; }
    int getArgumentCount() const { return args; }

    void addSupportedCoreAttributes(CAType ca)
    {
        supportedCoreAttributes.append(ca);
    }
    bool checkCoreAttributesType(ExpressionTree* et);

    long longCall(ExpressionTree* et, Operation* const ops[]) const;

    long hasAssignments(ExpressionTree* et, Operation* const ops[]) const;
    long isParentOf(ExpressionTree* et, Operation* const ops[]) const;
    long isChildOf(ExpressionTree* et, Operation* const ops[]) const;
    long isLeaf(ExpressionTree* et, Operation* const ops[]) const;
    long treeLevel(ExpressionTree* et, Operation* const ops[]) const;
    long isAccount(ExpressionTree* et, Operation* const ops[]) const;
    long isAnAccount(ExpressionTree* et, Operation* const ops[]) const;
    long isResource(ExpressionTree* et, Operation* const ops[]) const;
    long isAResource(ExpressionTree* et, Operation* const ops[]) const;
    long isMilestone(ExpressionTree* et, Operation* const ops[]) const;
    long isTask(ExpressionTree* et, Operation* const ops[]) const;
    long isATask(ExpressionTree* et, Operation* const ops[]) const;
    long isSubTaskOf(ExpressionTree* et, Operation* const ops[]) const;
    long isTaskOfProject(ExpressionTree* et, Operation* const ops[])
        const;
    long startsBefore(ExpressionTree* et, Operation* const ops[]) const;
    long startsAfter(ExpressionTree* et, Operation* const ops[]) const;
    long endsBefore(ExpressionTree* et, Operation* const ops[]) const;
    long endsAfter(ExpressionTree* et, Operation* const ops[]) const;
    long isAllocated(ExpressionTree* et, Operation* const ops[]) const;
    long isDutyOf(ExpressionTree* et, Operation* const ops[]) const;
    long isAllocatedToProject(ExpressionTree* et, Operation* const ops[])
        const;
    long isOnCriticalPath(ExpressionTree* et, Operation* const ops[]) const;

    /* Deprecated functions */
    long isTaskStatus(ExpressionTree* et, Operation* const ops[]) const;
    long containsTask(ExpressionTree* et, Operation* const ops[]) const;
    long isPlanAllocated(ExpressionTree* et, Operation* const ops[])
        const;
    long isActualAllocated(ExpressionTree* et, Operation* const ops[])
        const;

private:
    const CoreAttributes* findCoreAttributes(const CoreAttributes* ca,
                                             const QString& id) const;

    QString name;
    ExpressionTreeFunctionLongPtr longFunc;
    int args;
    QValueList<CAType> supportedCoreAttributes;
} ;

#endif

