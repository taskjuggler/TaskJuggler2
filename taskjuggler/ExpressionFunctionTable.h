/*
 * ExpressionFunctionTable.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _ExpressionFunctionTable_h_
#define _ExpressionFunctionTable_h_

#include <qdict.h>

#include "ExpressionTreeFunction.h"

class ExpressionFunctionTable
{
public:
    ExpressionFunctionTable();
    ~ExpressionFunctionTable() { }

    bool isKnownFunction(const QString& name) const
    {
        return functions[name] != 0;
    }
    ExpressionTreeFunction* getFunction(const QString& name) const
    {
        return functions[name];
    }
    int getArgumentCount(const QString& name) const
    {
        if (!functions[name])
            return -1;
        return functions[name]->getArgumentCount();
    }

private:
    void addFunc(const QString& name, const ExpressionTreeFunctionLongPtr func,
                 const int args);

    QDict<ExpressionTreeFunction> functions;
} ;

extern ExpressionFunctionTable EFT;

#endif

