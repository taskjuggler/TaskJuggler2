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

#ifndef _ExpressionTree_h_
#define _ExpressionTree_h_

#include <qstring.h>
#include <qdict.h>
#include <qptrlist.h>

class CoreAttributes;
class ExpressionTree;

class Operation
{
public:
	enum opType { Const = 1, Variable, Function, TaskId, ResourceId,
		AccountId, Not, And, Or };

	Operation(long v) : opt(Const), value(v) { }
	Operation(opType ot, const QString& n) : opt(ot), name(n) { }
	Operation(const QString& v) : opt(Variable), name(v) { }
	Operation(Operation* o1, opType o, Operation* o2 = 0)
		: opt(o)
	{
		ops.setAutoDelete(TRUE);
		ops.append(o1);
		ops.append(o2);
	}
	Operation(const QString n, QPtrList<Operation> args) :
	   opt(Function), name(n)
   	{
		ops = args;
		ops.setAutoDelete(TRUE);
   	}
	~Operation() { }

	long evalAsInt(ExpressionTree* et);
	QString evalAsString(ExpressionTree* et);

private:
	Operation() { } // don't use this

	long evalFunction(ExpressionTree* et);
	QString evalFunctionAsString(ExpressionTree* et);

	opType opt;
	long value;
	QString name;
	QPtrList<Operation> ops;
} ;

class ExpressionTree
{
public:
	ExpressionTree(Operation* op);
	~ExpressionTree() { }

	long evalAsInt(CoreAttributes* c)
   	{
		ca = c;
		return expression->evalAsInt(this);
   	}
	long resolve(const QString& symbol);

	void registerSymbol(const QString& symbol, long value)
	{
		symbolTable.insert(symbol, new long(value));
	}
	void clearSymbolTable() { symbolTable.clear(); }

	CoreAttributes* getCoreAttributes() { return ca; }

	static bool isFunction(const QString& name);

	static int arguments(const QString& name);

private:
	ExpressionTree() { }	// don't use this

	CoreAttributes* ca;
	QDict<long> symbolTable;
	Operation* expression;
	
	static QDict<int> funcArgc;
} ;

#endif
