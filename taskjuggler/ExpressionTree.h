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

#ifndef _ExpressionTree_h_
#define _ExpressionTree_h_

#include <time.h>

#include <qstring.h>
#include <qdict.h>
#include <qptrlist.h>

#include "ExpressionTreeFunction.h"

class CoreAttributes;
class ExpressionTree;
class Operation;

class ExpressionTree
{
public:
    ExpressionTree(const Operation* op);
    ExpressionTree(const ExpressionTree& et);
    ~ExpressionTree();

    long evalAsInt(const CoreAttributes* c);
    long resolve(const QString& symbol) const;

    void registerSymbol(const QString& symbol, long value)
    {
        symbolTable.insert(symbol, new long(value));
    }
    void clearSymbolTable() { symbolTable.clear(); }

    const CoreAttributes* getCoreAttributes() const { return ca; }

    static bool isFunction(const QString& name);

    static int arguments(const QString& name);

    ExpressionTreeFunction* getFunction(const QString& name) const
    {
        return functions[name];
    }
    
private:
    ExpressionTree() { }    // don't use this

    const CoreAttributes* ca;
    QDict<long> symbolTable;
    const Operation* expression;
    
    static QDict<ExpressionTreeFunction> functions;
} ;

#endif
