/*
 * ExpressionTree.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "ExpressionTree.h"
#include "Operation.h"
#include "CoreAttributes.h"
#include "Project.h"
#include "Task.h"
#include "Resource.h"
#include "Utility.h"
#include "debug.h"

// Dummy marco to mark all keywords of taskjuggler syntax
#define KW(a) a

QDict<ExpressionTreeFunction> ExpressionTree::functions;

ExpressionTree::ExpressionTree(const Operation* op) : expression(op)
{
    functions.setAutoDelete(TRUE);
    symbolTable.setAutoDelete(TRUE);
    if (functions.isEmpty())
    {
        ExpressionTreeFunction* etf = new ExpressionTreeFunction
            ("istask", &ExpressionTreeFunction::isTask, 1);
        functions.insert(etf->getName(), etf);
        etf = new ExpressionTreeFunction
            (KW("isTask"), &ExpressionTreeFunction::isTask, 1);
        functions.insert(etf->getName(), etf);
        
        etf = new ExpressionTreeFunction
            (KW("isatask"), &ExpressionTreeFunction::isATask, 0);
        functions.insert(etf->getName(), etf);
        etf = new ExpressionTreeFunction
            (KW("isATask"), &ExpressionTreeFunction::isATask, 0);
        
        functions.insert(etf->getName(), etf);
        etf = new ExpressionTreeFunction
            ("ismilestone", &ExpressionTreeFunction::isMilestone, 0);
        functions.insert(etf->getName(), etf);
        etf = new ExpressionTreeFunction
            (KW("isMilestone"), &ExpressionTreeFunction::isMilestone, 0);
        functions.insert(etf->getName(), etf);
        
        etf = new ExpressionTreeFunction
            ("istaskofproject", &ExpressionTreeFunction::isTaskOfProject, 1);
        functions.insert(etf->getName(), etf);
        etf = new ExpressionTreeFunction
            (KW("isTaskOfProject"), 
             &ExpressionTreeFunction::isTaskOfProject, 1);
        functions.insert(etf->getName(), etf);
        
        etf = new ExpressionTreeFunction
            ("isresource", &ExpressionTreeFunction::isResource, 1);
        functions.insert(etf->getName(), etf);
        etf = new ExpressionTreeFunction
            (KW("isResource"), &ExpressionTreeFunction::isResource, 1);
        functions.insert(etf->getName(), etf);
        
        etf = new ExpressionTreeFunction
            ("isaresource", &ExpressionTreeFunction::isAResource, 0);
        functions.insert(etf->getName(), etf);
        etf = new ExpressionTreeFunction
            (KW("isAResource"), &ExpressionTreeFunction::isAResource, 0);
        functions.insert(etf->getName(), etf);
        
        etf = new ExpressionTreeFunction
            ("isaccount", &ExpressionTreeFunction::isAccount, 1);
        functions.insert(etf->getName(), etf);
        etf = new ExpressionTreeFunction
            (KW("isAccount"), &ExpressionTreeFunction::isAccount, 1);
        functions.insert(etf->getName(), etf);
        
        etf = new ExpressionTreeFunction
            ("isanaccount", &ExpressionTreeFunction::isAnAccount, 0);
        functions.insert(etf->getName(), etf);
        etf = new ExpressionTreeFunction
            (KW("isAnAccount"), &ExpressionTreeFunction::isAnAccount, 0);
        functions.insert(etf->getName(), etf);
        
        etf = new ExpressionTreeFunction
            ("istaskstatus", &ExpressionTreeFunction::isTaskStatus, 2);
        functions.insert(etf->getName(), etf);
        etf = new ExpressionTreeFunction
            (KW("isTaskStatus"), &ExpressionTreeFunction::isTaskStatus, 2);
        functions.insert(etf->getName(), etf);
        
        etf = new ExpressionTreeFunction
            ("startsbefore", &ExpressionTreeFunction::startsBefore, 2);
        functions.insert(etf->getName(), etf);
        etf = new ExpressionTreeFunction
            (KW("startsBefore"), &ExpressionTreeFunction::startsBefore, 2);
        functions.insert(etf->getName(), etf);

        etf = new ExpressionTreeFunction
            ("startsafter", &ExpressionTreeFunction::startsAfter, 2);
        functions.insert(etf->getName(), etf);
        etf = new ExpressionTreeFunction
            (KW("startsAfter"), &ExpressionTreeFunction::startsAfter, 2);
        functions.insert(etf->getName(), etf);
        
        etf = new ExpressionTreeFunction
            ("endsbefore", &ExpressionTreeFunction::endsBefore, 2);
        functions.insert(etf->getName(), etf);
        etf = new ExpressionTreeFunction
            (KW("endsBefore"), &ExpressionTreeFunction::endsBefore, 2);
        functions.insert(etf->getName(), etf);
        
        etf = new ExpressionTreeFunction
            ("endsafter", &ExpressionTreeFunction::endsAfter, 2);
        functions.insert(etf->getName(), etf);
        etf = new ExpressionTreeFunction
            (KW("endsAfter"), &ExpressionTreeFunction::endsAfter, 2);
        functions.insert(etf->getName(), etf);
        
        etf = new ExpressionTreeFunction
            ("isparentof", &ExpressionTreeFunction::isParentOf, 1);
        functions.insert(etf->getName(), etf);
        etf = new ExpressionTreeFunction
            (KW("isParentOf"), &ExpressionTreeFunction::isParentOf, 1);
        functions.insert(etf->getName(), etf);
        
        etf = new ExpressionTreeFunction
            ("ischildof", &ExpressionTreeFunction::isChildOf, 1);
        functions.insert(etf->getName(), etf);
        etf = new ExpressionTreeFunction
            (KW("isChildOf"), &ExpressionTreeFunction::isChildOf, 1);
        functions.insert(etf->getName(), etf);

        etf = new ExpressionTreeFunction
            ("isleaf", &ExpressionTreeFunction::isLeaf, 0);
        functions.insert(etf->getName(), etf);
        etf = new ExpressionTreeFunction
            (KW("isLeaf"), &ExpressionTreeFunction::isLeaf, 0);
        functions.insert(etf->getName(), etf);
        
        etf = new ExpressionTreeFunction
            ("treelevel", &ExpressionTreeFunction::treeLevel, 0);
        functions.insert(etf->getName(), etf);
        etf = new ExpressionTreeFunction
            (KW("treeLevel"), &ExpressionTreeFunction::treeLevel, 0);
        functions.insert(etf->getName(), etf);
        
        etf = new ExpressionTreeFunction
            ("isallocated", &ExpressionTreeFunction::isAllocated, 3);
        functions.insert(etf->getName(), etf);
        etf = new ExpressionTreeFunction
            (KW("isAllocated"), &ExpressionTreeFunction::isAllocated, 3);
        functions.insert(etf->getName(), etf);
        
        etf = new ExpressionTreeFunction
            ("isdutyof", &ExpressionTreeFunction::isDutyOf, 2);
        functions.insert(etf->getName(), etf);
        etf = new ExpressionTreeFunction
            (KW("isDutyOf"), &ExpressionTreeFunction::isDutyOf, 2);
        functions.insert(etf->getName(), etf);
        
        etf = new ExpressionTreeFunction
            ("isallocatedtoproject", &ExpressionTreeFunction::isAllocated, 4);
        functions.insert(etf->getName(), etf);
        etf = new ExpressionTreeFunction
            (KW("isAllocatedToProject"), 
             &ExpressionTreeFunction::isAllocatedToProject, 4);
        functions.insert(etf->getName(), etf);

        /* The following functions are for legacy support only. Their
         * use is discouraged since they will disappear some day. */
        etf = new ExpressionTreeFunction
            (KW("isplanallocated"), 
             &ExpressionTreeFunction::isPlanAllocated, 3);
        functions.insert(etf->getName(), etf);
        etf = new ExpressionTreeFunction
            (KW("isactualallocated"),
             &ExpressionTreeFunction::isActualAllocated, 3);
        functions.insert(etf->getName(), etf);
        etf = new ExpressionTreeFunction
            (KW("issubtaskof"), &ExpressionTreeFunction::isSubTaskOf, 1);
        functions.insert(etf->getName(), etf);
        etf = new ExpressionTreeFunction
            (KW("containstask"), &ExpressionTreeFunction::containsTask, 1);
        functions.insert(etf->getName(), etf);
    }
}

ExpressionTree::ExpressionTree(const ExpressionTree& et)
{
    ca = et.ca;
    symbolTable = et.symbolTable;
    expression = new Operation(*et.expression);
}

ExpressionTree::~ExpressionTree()
{
    delete expression;
}

long 
ExpressionTree::evalAsInt(const CoreAttributes* c)
{
    ca = c;
    return expression->evalAsInt(this);
}

long
ExpressionTree::resolve(const QString& symbol) const
{
    return symbolTable[symbol] != 0 ? *(symbolTable[symbol]) : 0;
}

bool
ExpressionTree::isFunction(const QString& name)
{
    return functions[name];
}

int
ExpressionTree::arguments(const QString& name)
{
    return functions[name]->getArgumentCount();
}

