/*
 * ExpressionTree.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
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
#include "ExpressionParser.h"
#include "ExpressionFunctionTable.h"
#include "TjMessageHandler.h"
#include "tjlib-internal.h"

// do not move this into the header, it expands the library image size
ExpressionTree::ExpressionTree(const Operation* op) :
    ca(0),
    symbolTable(),
    expression(op),
    evalErrorFlag(false),
    defFileName(),
    defLineNo(0)
{
    symbolTable.setAutoDelete(true);
}

// do not move this into the header, it expands the library image size
ExpressionTree::ExpressionTree(const ExpressionTree& et) :
    ca(et.ca),
    symbolTable(et.symbolTable),
    expression(new Operation(*et.expression)),
    evalErrorFlag(false),
    defFileName(),
    defLineNo(0)
{
}

// do not move this into the header, it expands the library image size
ExpressionTree::ExpressionTree() :
    ca(0),
    symbolTable(),
    expression(0),
    evalErrorFlag(false),
    defFileName(),
    defLineNo(0)
{
    symbolTable.setAutoDelete(true);
}

ExpressionTree::~ExpressionTree()
{
    delete expression;
}

bool
ExpressionTree::setTree(const QString& expr, const Project* proj)
{
    ExpressionParser parser(expr, proj);
    return (expression = parser.parse()) != 0;
}

long
ExpressionTree::evalAsInt(const CoreAttributes* c)
{
    evalErrorFlag = false;
    ca = c;
    long val = expression->evalAsInt(this);

    return val;
}

long
ExpressionTree::resolve(const QString& symbol)
{
    if (!symbolTable[symbol])
    {
        errorMessage(i18n("Unknown identifier '%1' in logical expression")
                     .arg(symbol));
        return 0;
    }

    return *(symbolTable[symbol]);
}

void
ExpressionTree::errorMessage(const QString& msg)
{
    TJMH.errorMessage(msg, defFileName, defLineNo);
    evalErrorFlag = true;
}

