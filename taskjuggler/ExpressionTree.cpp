/*
 * ExpressionTree.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "ExpressionTree.h"
#include "CoreAttributes.h"
#include "Project.h"
#include "Task.h"
#include "Utility.h"

// Dummy marco to mark all keywords of taskjuggler syntax
#define KW(a) a

QDict<int> ExpressionTree::funcArgc;

long
Operation::evalAsInt(ExpressionTree* et)
{
	switch (opt)
	{
	case Const:
		return value;
	case Variable:
	case Id:
		return et->resolve(name);
	case Function:
		return evalFunction(et);
	case Date:
		return value;
	case Not:
		return !ops.at(0)->evalAsInt(et);
	case And:
		return ops.at(0)->evalAsInt(et) && ops.at(1)->evalAsInt(et);
	case Or:
		return ops.at(0)->evalAsInt(et) || ops.at(1)->evalAsInt(et);
	default:
		qFatal("Operation::evalAsInt: "
			   "Unknown opType %d (name: %s)", opt, name.ascii());
		return 0;
	}
}

time_t
Operation::evalAsTime(ExpressionTree* et)
{
	switch(opt)
	{
	case Const:
	case Date:
		return value;
	case Variable:
	case Id:
		return et->resolve(name);
	case Function:
		return evalFunction(et);
	default:
		qFatal("Operation::evalAsTime: "
			   "Unknown opType %d (name: %s)", opt, name.ascii());
		return 0;
	}
}

QString
Operation::evalAsString(ExpressionTree* et)
{
	switch(opt)
	{
	case Const:
		return QString("%1").arg(value);
	case Function:
		return evalFunctionAsString(et);
	case Date:
		return time2date(value);
	case Id:
		return name;
	default:
		qFatal("Operation::evalAsString: "
			   "Unknown opType %d (name: %s)", opt, name.ascii());
		return QString::null;
	}
}

long
Operation::evalFunction(ExpressionTree* et)
{
	if (name == "istask")
	{
		return strcmp(et->getCoreAttributes()->getType(), "Task") == 0
			&& et->getCoreAttributes()->getId() ==
			ops.at(0)->evalAsString(et);
	}
	else if (name == "issubtaskof")
	{
		if (strcmp(et->getCoreAttributes()->getType(), "Task") != 0)
			return 0;
		Task* p;
		if ((p = et->getCoreAttributes()->getProject()->getTask
			 (ops.at(0)->evalAsString(et))) == 0)
			return 0;
		return p->isSubTask((Task*) et->getCoreAttributes());
	}
	else if (name == "containstask")
	{
		if (strcmp(et->getCoreAttributes()->getType(), "Task") != 0)
			return 0;
		Task* st;
		if ((st = et->getCoreAttributes()->getProject()->getTask
			 (ops.at(0)->evalAsString(et))) == 0)
			return 0;
		return ((Task*) et->getCoreAttributes())->isSubTask(st);
	}
	else if (name == "ismilestone")
	{
		if (strcmp(et->getCoreAttributes()->getType(), "Task") != 0)
			return 0;
		return ((Task*) et->getCoreAttributes())->isMilestone();
	}
	else if (name == "isresource")
	{
		return strcmp(et->getCoreAttributes()->getType(), "Resource") == 0
			&& et->getCoreAttributes()->getId() ==
			ops.at(0)->evalAsString(et);
	}
	else if (name == "isaccount")
	{
		return strcmp(et->getCoreAttributes()->getType(), "Account") == 0
			&& et->getCoreAttributes()->getId() ==
			ops.at(0)->evalAsString(et);
	}
	else if (name == "isplanallocated")
	{
		if (strcmp(et->getCoreAttributes()->getType(), "Resource") != 0)
			qFatal("Operation::evalFunction: isplanallocated called for "
				   "non-resource");
		return ((Resource*) et->getCoreAttributes())->isAllocated
			(Task::Plan, Interval(ops.at(1)->evalAsTime(et), 
								  ops.at(2)->evalAsTime(et)), 
			 ops.at(0)->evalAsString(et));
	}
	else if (name == "isactualallocated")
	{
		if (strcmp(et->getCoreAttributes()->getType(), "Resource") != 0)
			qFatal("Operation::evalFunction: isactualallocated called for "
				   "non-resource");
		return ((Resource*) et->getCoreAttributes())->isAllocated
			(Task::Actual, Interval(ops.at(1)->evalAsTime(et), 
									ops.at(2)->evalAsTime(et)), 
			 ops.at(0)->evalAsString(et));
	}
	else
		qFatal("Unknown function %s", name.data());	

	return 0;
}

QString
Operation::evalFunctionAsString(ExpressionTree* )
{
	// There are no functions yet that return a string.
	return QString::null;
}

ExpressionTree::ExpressionTree(Operation* op) : expression(op)
{
	symbolTable.setAutoDelete(TRUE);
	if (funcArgc.isEmpty())
	{
		funcArgc.insert(KW("istask"), new int(1));
		funcArgc.insert(KW("issubtaskof"), new int(1));
		funcArgc.insert(KW("containstask"), new int(1));
		funcArgc.insert(KW("ismilestone"), new int(0));
		funcArgc.insert(KW("isresource"), new int(1));
		funcArgc.insert(KW("isaccount"), new int(1));
		funcArgc.insert(KW("isplanallocated"), new int(3));
		funcArgc.insert(KW("isactualallocated"), new int(3));
	}
}

long
ExpressionTree::resolve(const QString& symbol)
{
	return symbolTable[symbol] != 0 ? *(symbolTable[symbol]) : 0;
}

bool
ExpressionTree::isFunction(const QString& name)
{
	return funcArgc[name];
}

int
ExpressionTree::arguments(const QString& name)
{
	return *funcArgc[name];
}

