/*
 * ExpressionTreeFunction.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@suse.de>
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
    (const ExpressionTree*, Operation* const ops[]) const;
    
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

    long longCall(const ExpressionTree* et, Operation* const ops[]) const;

    long isParentOf(const ExpressionTree* et, Operation* const ops[]) const;
    long isChildOf(const ExpressionTree* et, Operation* const ops[]) const;
    long isLeaf(const ExpressionTree* et, Operation* const ops[]) const;
    long treeLevel(const ExpressionTree* et, Operation* const ops[]) const;
    long isAccount(const ExpressionTree* et, Operation* const ops[]) const;
    long isAnAccount(const ExpressionTree* et, Operation* const ops[]) const;
    long isResource(const ExpressionTree* et, Operation* const ops[]) const;
    long isAResource(const ExpressionTree* et, Operation* const ops[]) const;
    long isMilestone(const ExpressionTree* et, Operation* const ops[]) const;
    long isTask(const ExpressionTree* et, Operation* const ops[]) const;
    long isATask(const ExpressionTree* et, Operation* const ops[]) const;
    long isSubTaskOf(const ExpressionTree* et, Operation* const ops[]) const;
    long isTaskOfProject(const ExpressionTree* et, Operation* const ops[])
        const;
    long startsBefore(const ExpressionTree* et, Operation* const ops[]) const;
    long startsAfter(const ExpressionTree* et, Operation* const ops[]) const;
    long endsBefore(const ExpressionTree* et, Operation* const ops[]) const;
    long endsAfter(const ExpressionTree* et, Operation* const ops[]) const;
    long isAllocated(const ExpressionTree* et, Operation* const ops[]) const;
    long isDutyOf(const ExpressionTree* et, Operation* const ops[]) const;
    long isAllocatedToProject(const ExpressionTree* et, Operation* const ops[])
        const;
   
    /* Deprecated functions */ 
    long isTaskStatus(const ExpressionTree* et, Operation* const ops[]) const;
    long containsTask(const ExpressionTree* et, Operation* const ops[]) const;
    long isPlanAllocated(const ExpressionTree* et, Operation* const ops[])
        const;
    long isActualAllocated(const ExpressionTree* et, Operation* const ops[])
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

