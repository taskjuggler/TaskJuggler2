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
#include "ExpressionParser.h"
#include "ExpressionFunctionTable.h"
#include "TjMessageHandler.h"

// Dummy marco to mark all keywords of taskjuggler syntax
#define KW(a) a

ExpressionTree::ExpressionTree(const Operation* op) : expression(op)
{
    symbolTable.setAutoDelete(TRUE);
    evalErrorFlag = FALSE;
}

ExpressionTree::ExpressionTree(const ExpressionTree& et)
{
    ca = et.ca;
    symbolTable = et.symbolTable;
    expression = new Operation(*et.expression);
    evalErrorFlag = FALSE;
}

ExpressionTree::ExpressionTree()
{
    symbolTable.setAutoDelete(TRUE);
    evalErrorFlag = FALSE;
}

ExpressionTree::~ExpressionTree()
{
    delete expression;
}

bool
ExpressionTree::setTree(const QString& expr, const Project* proj)
{
    ExpressionParser parser;
    return (expression = parser.parse(expr, proj)) != 0;
}

long
ExpressionTree::evalAsInt(const CoreAttributes* c)
{
    evalErrorFlag = FALSE;
    ca = c;
    long val = expression->evalAsInt(this);

    return val;
}

long
ExpressionTree::resolve(const QString& symbol) const
{
    return symbolTable[symbol] != 0 ? *(symbolTable[symbol]) : 0;
}

void
ExpressionTree::errorMessage(const char* msg, ...)
{
    va_list ap;
    char* buf = new char[32 + 2 * strlen(msg)];
    va_start(ap, msg);
    vsnprintf(buf, 1024, msg, ap);
    va_end(ap);

    TJMH.errorMessage(buf, defFileName, defLineNo);
    delete [] buf;
    evalErrorFlag = TRUE;
}

