/*
 * Operation.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _Operation_h_
#define _Operation_h_

#include <time.h>

#include <qstring.h>
#include <qptrlist.h>

class QString;

class ExpressionTree;
class Operation;

class Operation
{
public:
	enum opType { Const = 1, Variable, Function, Id, Date, String, 
		Not, And, Or };

	Operation(long v) : opt(Const), value(v), opsCount(0) { }
	Operation(opType ot, const QString& n) : opt(ot), name(n), opsCount(0) { }
	Operation(opType ot, long v) : opt(ot), value(v), opsCount(0) { }
	Operation(opType ot, const QString& n, long v) :
	   	opt(ot), value(v), name(n), opsCount(0) { }
	Operation(const QString& v) : opt(Variable), name(v), opsCount(0) { }
	Operation(Operation* o1, opType o, Operation* o2 = 0)
		: opt(o)
	{
		ops = new Operation*[2];
		ops[0] = o1;
		ops[1] = o2;
		opsCount = 2;
	}
	Operation(const QString& n, Operation* o1) : name(n)
	{
		opt = Function;
		ops = new Operation*[1];
		ops[0] = o1;
		opsCount = 1;
	}
	Operation(const QString& n, Operation* o1, Operation* o2) : name(n)
	{
		opt = Function;
		ops = new Operation*[2];
		ops[0] = o1;
		ops[1] = o2;
		opsCount = 2;
	}
	Operation(const QString& n, Operation* args[], int c) :
	   opt(Function), name(n), ops(args), opsCount(c) { }
	~Operation();

	long evalAsInt(const ExpressionTree* et) const;
	time_t evalAsTime(const ExpressionTree* et) const;
	QString evalAsString(const ExpressionTree* et) const;

	QString debugString();
	
private:
	Operation() { } // don't use this

	long evalFunction(const ExpressionTree* et) const;
	QString evalFunctionAsString(const ExpressionTree* et) const;

	opType opt;
	long value;
	QString name;
	Operation** ops;
	int opsCount;
} ;

#endif
