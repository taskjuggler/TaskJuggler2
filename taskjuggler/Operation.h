/*
 * Operation.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
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
        Not, And, Or,
        Greater, Smaller, Equal, GreaterOrEqual, SmallerOrEqual };

    Operation(long v) :
        opt(Const),
        value(v),
        name(),
        ops(0),
        opsCount(0),
        valid(false)
    { }

    Operation(opType ot, const QString& n) :
        opt(ot),
        value(0),
        name(n),
        ops(0),
        opsCount(0),
        valid(false)
    { }

    Operation(opType ot, long v) :
        opt(ot),
        value(v),
        name(),
        ops(0),
        opsCount(0),
        valid(false)
    { }

    Operation(opType ot, const QString& n, long v) :
        opt(ot),
        value(v),
        name(n),
        ops(0),
        opsCount(0),
        valid(false)
    { }

    Operation(const QString& v) :
        opt(Variable),
        value(0),
        name(v),
        ops(0),
        opsCount(0),
        valid(false)
    { }

    Operation(Operation* o1, opType ot, Operation* o2 = 0) :
        opt(ot),
        value(0),
        name(),
        ops(new Operation*[2]),
        opsCount(2),
        valid(false)
    {
        ops[0] = o1;
        ops[1] = o2;

    }
    Operation(const QString& n, Operation* o1) :
        opt(Function),
        value(0),
        name(n),
        ops(new Operation*[1]),
        opsCount(1),
        valid(false)
    {
        ops[0] = o1;
    }

    Operation(const QString& n, Operation* o1, Operation* o2) :
        opt(Function),
        value(0),
        name(n),
        ops(new Operation*[2]),
        opsCount(2),
        valid(false)
    {
        ops[0] = o1;
        ops[1] = o2;
    }

    Operation(const QString& n, Operation* args[], int c) :
        opt(Function),
        value(0),
        name(n),
        ops(args),
        opsCount(c),
        valid(false)
    { }

    Operation(const Operation& op);

    ~Operation();

    long evalAsInt(ExpressionTree* et) const;
    time_t evalAsTime(ExpressionTree* et) const;
    QString evalAsString(ExpressionTree* et) const;

    void setValid(bool v = true)
    {
        valid = v;
    }
    bool isValid() const { return valid; }

    QString debugString();

private:
    long evalFunction(ExpressionTree* et) const;
    QString evalFunctionAsString(ExpressionTree* et) const;

    opType opt;
    long value;
    QString name;
    Operation** ops;
    int opsCount;
    bool valid;
} ;

#endif

