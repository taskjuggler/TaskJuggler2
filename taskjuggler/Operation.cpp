/*
 * Operation.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "Operation.h"
#include "ExpressionTree.h"
#include "Utility.h"

Operation::~Operation()
{
    for (int i = 0 ; i < opsCount; ++i)
        delete ops[i];
    if (opsCount > 0)
        delete [] ops;
}

long
Operation::evalAsInt(const ExpressionTree* et) const
{
    switch (opt)
    {
    case Const:
        return value;
    case Variable:
    case Id:
        return et->resolve(name);
    case Function:
        return evalFunction(et);
    case Date:
        return value;
    case String:
        return name.toLong();
    case Not:
        return !ops[0]->evalAsInt(et);
    case And:
        return ops[0]->evalAsInt(et) && ops[1]->evalAsInt(et);
    case Or:
        return ops[0]->evalAsInt(et) || ops[1]->evalAsInt(et);
    default:
        qFatal("Operation::evalAsInt: "
               "Unknown opType %d (name: %s)", opt, name.ascii());
        return 0;
    }
}

time_t
Operation::evalAsTime(const ExpressionTree* et) const
{
    switch(opt)
    {
    case Const:
    case Date:
        return value;
    case String:
        return date2time(name);
    case Variable:
    case Id:
        return et->resolve(name);
    case Function:
        return evalFunction(et);
    default:
        qFatal("Operation::evalAsTime: "
               "Unknown opType %d (name: %s)", opt, name.ascii());
        return 0;
    }
}

QString
Operation::evalAsString(const ExpressionTree* et) const
{
    switch(opt)
    {
    case Const:
        return QString("%1").arg(value);
    case Function:
        return evalFunctionAsString(et);
    case Date:
        return time2date(value);
    case Id:
        return name;
    case String:
        return name;
    default:
        qFatal("Operation::evalAsString: "
               "Unknown opType %d (name: %s)", opt, name.ascii());
        return QString::null;
    }
}

long
Operation::evalFunction(const ExpressionTree* et) const
{
    if (et->getFunction(name))
    {
        return et->getFunction(name)->longCall(et, ops);
    }
    else
        qFatal("Unknown function %s", name.data()); 

    return 0;
}

QString
Operation::evalFunctionAsString(const ExpressionTree* ) const
{
    // There are no functions yet that return a string.
    return QString::null;
}

QString
Operation::debugString()
{
    QString res;
    switch(opt)
    {
        case Const:
            res.sprintf("Const:%ld", value);
            break;
        case Variable:
            res.sprintf("Variable:%s", name.latin1());
            break;
        case Function:
            res.sprintf("Function:%s", name.latin1());
            break;
        case Id:
            res.sprintf("Id:%s", name.latin1());
            break;
        case Date:
            res.sprintf("Date:%s", name.latin1());
            break;
        case String:
            res = name;
            break;
        case Not:
            res = "Not";
            break;
        case And:
            res = "And";
            break;
        case Or:
            res = "Or";
            break;
        default:
            res = "Unknown";
    }
    return res;
}

