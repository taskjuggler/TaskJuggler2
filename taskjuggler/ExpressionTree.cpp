/*
 * ExpressionTree.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
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
	symbolTable.setAutoDelete(TRUE);
	if (functions.isEmpty())
	{
		ExpressionTreeFunction* etf = new ExpressionTreeFunction
			(KW("istask"), &ExpressionTreeFunction::isTask, 1);
		functions.insert(etf->getName(), etf);
		etf = new ExpressionTreeFunction
			(KW("issubtaskof"), &ExpressionTreeFunction::isSubTaskOf, 1);
		functions.insert(etf->getName(), etf);
		etf = new ExpressionTreeFunction
			(KW("containstask"), &ExpressionTreeFunction::containsTask, 1);
		functions.insert(etf->getName(), etf);
		etf = new ExpressionTreeFunction
			(KW("ismilestone"), &ExpressionTreeFunction::isMilestone, 0);
		functions.insert(etf->getName(), etf);
		etf = new ExpressionTreeFunction
			(KW("isresource"), &ExpressionTreeFunction::isResource, 1);
		functions.insert(etf->getName(), etf);
		etf = new ExpressionTreeFunction
			(KW("isaccount"), &ExpressionTreeFunction::isAccount, 1);
		functions.insert(etf->getName(), etf);
		etf = new ExpressionTreeFunction
			(KW("istaskstatus"), &ExpressionTreeFunction::isTaskStatus, 2);
		functions.insert(etf->getName(), etf);
		etf = new ExpressionTreeFunction
			(KW("startsbefore"), &ExpressionTreeFunction::startsBefore, 2);
		functions.insert(etf->getName(), etf);
		etf = new ExpressionTreeFunction
			(KW("startsafter"), &ExpressionTreeFunction::startsAfter, 2);
		functions.insert(etf->getName(), etf);
		etf = new ExpressionTreeFunction
			(KW("endsbefore"), &ExpressionTreeFunction::endsBefore, 2);
		functions.insert(etf->getName(), etf);
		etf = new ExpressionTreeFunction
			(KW("endsafter"), &ExpressionTreeFunction::endsAfter, 2);
		functions.insert(etf->getName(), etf);

		/* The following functions are for legacy support only. Their
		 * use is discouraged since they will disappear some day.
		 */
		etf = new ExpressionTreeFunction
			(KW("isplanallocated"), 
			 &ExpressionTreeFunction::isPlanAllocated, 3);
		functions.insert(etf->getName(), etf);
		etf = new ExpressionTreeFunction
			(KW("isactualallocated"),
			 &ExpressionTreeFunction::isActualAllocated, 3);
		functions.insert(etf->getName(), etf);
	}
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

