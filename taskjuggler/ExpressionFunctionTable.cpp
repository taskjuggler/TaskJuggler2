/*
 * ExpressionFunctionTable.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006
 * by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "ExpressionFunctionTable.h"

#include "tjlib-internal.h"

ExpressionFunctionTable EFT;

ExpressionFunctionTable::ExpressionFunctionTable() :
    functions()
{
    functions.setAutoDelete(TRUE);

    addFunc(KW("hasAssignments"), &ExpressionTreeFunction::hasAssignments, 2);
    addFunc(KW("isTask"), &ExpressionTreeFunction::isTask, 1);
    addFunc(KW("isATask"), &ExpressionTreeFunction::isATask, 0);
    addFunc(KW("isMilestone"), &ExpressionTreeFunction::isMilestone, 0);
    addFunc(KW("isTaskOfProject"), &ExpressionTreeFunction::isTaskOfProject, 1);
    addFunc(KW("isResource"), &ExpressionTreeFunction::isResource, 1);
    addFunc(KW("isAResource"), &ExpressionTreeFunction::isAResource, 0);
    addFunc(KW("isAccount"), &ExpressionTreeFunction::isAccount, 1);
    addFunc(KW("isAnAccount"), &ExpressionTreeFunction::isAnAccount, 0);
    addFunc(KW("isTaskStatus"), &ExpressionTreeFunction::isTaskStatus, 2);
    addFunc(KW("startsBefore"), &ExpressionTreeFunction::startsBefore, 2);
    addFunc(KW("startsAfter"), &ExpressionTreeFunction::startsAfter, 2);
    addFunc(KW("endsBefore"), &ExpressionTreeFunction::endsBefore, 2);
    addFunc(KW("endsAfter"), &ExpressionTreeFunction::endsAfter, 2);
    addFunc(KW("isParentOf"), &ExpressionTreeFunction::isParentOf, 1);
    addFunc(KW("isChildOf"), &ExpressionTreeFunction::isChildOf, 1);
    addFunc(KW("isLeaf"), &ExpressionTreeFunction::isLeaf, 0);
    addFunc(KW("treeLevel"), &ExpressionTreeFunction::treeLevel, 0);
    addFunc(KW("isAllocated"), &ExpressionTreeFunction::isAllocated, 3);
    addFunc(KW("isDutyOf"), &ExpressionTreeFunction::isDutyOf, 2);
    addFunc(KW("isAllocatedToProject"),
            &ExpressionTreeFunction::isAllocatedToProject, 4);
    addFunc(KW("isOnCriticalPath"),
            &ExpressionTreeFunction::isOnCriticalPath, 1);

    /* The following functions are for legacy support only. Their
     * use is discouraged since they will disappear some day. */
    addFunc(KW("isPlanAllocated"), &ExpressionTreeFunction::isPlanAllocated, 3);
    addFunc(KW("isActualAllocated"),
            &ExpressionTreeFunction::isActualAllocated, 3);
    addFunc(KW("isSubtaskOf"), &ExpressionTreeFunction::isSubTaskOf, 1);
    addFunc(KW("containsTask"), &ExpressionTreeFunction::containsTask, 1);
}

void
ExpressionFunctionTable::addFunc(const QString& name,
                                 const ExpressionTreeFunctionLongPtr func,
                                 const int args)
{
    functions.insert(name, new ExpressionTreeFunction(name, func, args));
    functions.insert(name.lower(), new ExpressionTreeFunction (name, func,
                                                               args));
}
