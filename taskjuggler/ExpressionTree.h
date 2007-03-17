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

#ifndef _ExpressionTree_h_
#define _ExpressionTree_h_

#include <time.h>

#include <qdict.h>
#include <qptrlist.h>

#include "ExpressionTreeFunction.h"

class CoreAttributes;
class ExpressionTree;
class Operation;
class Project;

/**
 * @short This class represents the logical expressions that can be used to
 * filter @see CoreAttributesList objects.
 * @author Chris Schlaeger <cs@kde.org>
 * @descr To filter certain elements out of CoreAttributes lists one needs to
 * specify a logical expression that describes the elements that should remain
 * in the list. The ExpressionTree stores such a logical expression. To filter
 * out the unwanted elements the ExpressionTree is evaluated against each of
 * the elements. The attributes of the element can be referenced in the
 * expression.
 */
class ExpressionTree
{
public:
    /**
     * Use this constructor when you have rolled your own operation tree
     * already.
     * @ param op the root of the operation tree
     */
    ExpressionTree(const Operation* op);
    /**
     * The usual copy constructor.
     */
    ExpressionTree(const ExpressionTree& et);
    /**
     * The default constructor. When you use this, you need to call setTree()
     * later on to assign an expression to the object.
     */
    ExpressionTree();
    /**
     * Destructor
     */
    ~ExpressionTree();

    /**
     * Use this fuction to assign a new expression to the object.
     * @param expr is a logical expresion in text form.
     * @param proj is a pointer to the project object.
     */
    bool setTree(const QString& expr, const Project* proj);

    void setDefLocation(const QString file, int line)
    {
        defFileName = file;
        defLineNo = line;
    }

    long evalAsInt(const CoreAttributes* c);
    long resolve(const QString& symbol);

    void setErrorFlag(bool flag = true)
    {
        evalErrorFlag = flag;
    }
    bool getErrorFlag() const
    {
        return evalErrorFlag;
    }

    void registerSymbol(const QString& symbol, long value)
    {
        symbolTable.insert(symbol, new long(value));
    }
    void clearSymbolTable() { symbolTable.clear(); }

    const CoreAttributes* getCoreAttributes() const { return ca; }

    void errorMessage(const char* msg, ...);

private:
    void generateFunctionTable();
    const CoreAttributes* ca;
    QDict<long> symbolTable;
    const Operation* expression;
    bool evalErrorFlag;

    QString defFileName;
    int defLineNo;
} ;

#endif
