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

long
Operation::eval(ExpressionTree* et)
{
	switch (opt)
	{
	case Const:
		return value;
	case Variable:
		return et->resolve(variable);
	case Not:
		return !op1->eval(et);
	case And:
		return op1->eval(et) && op2->eval(et);
	case Or:
		return op1->eval(et) || op2->eval(et);
	default:
		qFatal("Unknown opType");
	}
	return 0;	// should never be reached
}

long
ExpressionTree::resolve(const QString& symbol)
{
	return symbolTable[symbol] != 0 ? *(symbolTable[symbol]) : 0;
}
