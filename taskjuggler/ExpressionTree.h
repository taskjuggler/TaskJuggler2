/*
 * ProjectFile.cpp - TaskJuggler
 *
 * Copyright (c) 2001 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _ExpressionTree_h_
#define _ExpressionTree_h_

class Operation
{
public:
	enum opType { Const, Variable, Not, And, Or };

	Operation(long v) : opt(Const), value(v) { }
	Operation(const QString& v) : opt(Variable), variable(v) { }
	Operation(Operation* o1, opType o, Operation* o2 = 0)
		: opt(o), op1(o1), op2(o2) { }
	~Operation() { }

private:
	Operation() { } // don't use this

	opType opt;
	long value;
	QString variable;
	Operation* op1;
	Operation* op2;
} ;

class ExpressionTree
{
public:
	ExpressionTree(Operation* op) : expression(op) { }
	~ExpressionTree() { }

private:
	ExpressionTree() { }	// don't use this

	Operation* expression;
} ;

#endif
