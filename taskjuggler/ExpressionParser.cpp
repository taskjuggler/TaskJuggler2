/*
 * ExpressionParser.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "ExpressionParser.h"
#include "Project.h"
#include "Operation.h"
#include "Utility.h"
#include "debug.h"
#include "ExpressionTree.h"
#include "tjlib-internal.h"
#include "ExpressionFunctionTable.h"

Operation*
ExpressionParser::parse()
{
    Operation* op( 0 );

    if (tokenizer.open())
    {
        op = parseLogicalExpression(0);
        if (!tokenizer.close())
        {
            delete op;
            op = 0;
        }
    }
    return op;
}

Operation*
ExpressionParser::parseLogicalExpression(int precedence)
{
    Operation* op;
    QString token;
    TokenType tt;

    tt = tokenizer.nextToken(token);
    if (DEBUGEX(5))
        qDebug("parseLogicalExpression(%d): %s", precedence, token.latin1());
    if (tt == ID || tt == ABSOLUTE_ID)
    {
        QString lookAhead;
        if ((tt = tokenizer.nextToken(lookAhead)) == LBRACKET)
        {
            if (EFT.isKnownFunction(token))
            {
                if ((op = parseFunctionCall(token)) == 0)
                {
                    if (DEBUGEX(5))
                        qDebug("exit after function call");
                    return 0;
                }
            }
            else
            {
                errorMessage(i18n("Function '%1' is unknown").arg(token));
                return 0;
            }
        }
        else
        {
            tokenizer.returnToken(tt, lookAhead);
            /* We can't test here, whether the ID is known or not. So this has
             * to be checked at evaluation time. */
            op = new Operation(Operation::Id, token);
        }
    }
    else if (tt == STRING)
    {
        op = new Operation(Operation::String, token);
    }
    else if (tt == DATE)
    {
        time_t date;
        if ((date = date2time(token)) == 0)
        {
            errorMessage("%s", getUtilityError().latin1());
            return 0;
        }
        else
            op = new Operation(Operation::Date, date);
    }
    else if (tt == INTEGER)
    {
        op = new Operation(token.toLong());
    }
    else if (tt == TILDE)
    {
        if ((op = parseLogicalExpression(1)) == 0)
        {
            if (DEBUGEX(5))
                qDebug("exit after NOT");
            return 0;
        }
        op = new Operation(op, Operation::Not);
    }
    else if (tt == LBRACKET)
    {
        if ((op = parseLogicalExpression(0)) == 0)
        {
            if (DEBUGEX(5))
                qDebug("exit after ()");
            return 0;
        }
        if ((tt = tokenizer.nextToken(token)) != RBRACKET)
        {
            errorMessage(i18n("')' expected"));
            delete op;
            return 0;
        }
    }
    else
    {
        errorMessage(i18n("Logical expression expected"));
        return 0;
    }

    if (precedence < 1)
    {
        tt = tokenizer.nextToken(token);
        if (DEBUGEX(5))
            qDebug("Second operator %s", token.latin1());
        if (tt == AND)
        {
            Operation* op2 = parseLogicalExpression(0);
            op = new Operation(op, Operation::And, op2);
        }
        else if (tt == OR)
        {
            Operation* op2 = parseLogicalExpression(0);
            op = new Operation(op, Operation::Or, op2);
        }
        else if (tt == GREATER)
        {
            Operation* op2 = parseLogicalExpression(0);
            op = new Operation(op, Operation::Greater, op2);
        }
        else if (tt == SMALLER)
        {
            Operation* op2 = parseLogicalExpression(0);
            op = new Operation(op, Operation::Smaller, op2);
        }
        else if (tt == EQUAL)
        {
            Operation* op2 = parseLogicalExpression(0);
            op = new Operation(op, Operation::Equal, op2);
        }
        else if (tt == GREATEROREQUAL)
        {
            Operation* op2 = parseLogicalExpression(0);
            op = new Operation(op, Operation::GreaterOrEqual, op2);
        }
        else if (tt == SMALLEROREQUAL)
        {
            Operation* op2 = parseLogicalExpression(0);
            op = new Operation(op, Operation::SmallerOrEqual, op2);
        }
        else
            tokenizer.returnToken(tt, token);
     }

    if (DEBUGEX(5))
        qDebug("exit default");
    return op;
}

Operation*
ExpressionParser::parseFunctionCall(const QString& name)
{
    QString token;
    TokenType tt;

    QPtrList<Operation> args;
    for (int i = 0; i < EFT.getArgumentCount(name); i++)
    {
        if (DEBUGEX(5))
            qDebug("Reading function '%s' arg %d", name.latin1(), i);
        Operation* op;
        if ((op = parseLogicalExpression(0)) == 0)
            return 0;
        args.append(op);
        if ((i < EFT.getArgumentCount(name) - 1) &&
            tokenizer.nextToken(token) != COMMA)
        {
            errorMessage(i18n("Comma expected. "
                              "Function '%1' needs %2 arguments.")
                         .arg(name).arg(EFT.getArgumentCount(name)));
            return 0;
        }
    }
    if ((tt = tokenizer.nextToken(token)) != RBRACKET)
    {
        errorMessage(i18n("')' expected"));
        return 0;
    }
    Operation** argsArr = new Operation*[args.count()];
    int i = 0;
    for (QPtrListIterator<Operation> oli(args); *oli != 0; ++oli)
        argsArr[i++] = *oli;
    if (DEBUGEX(5))
        qDebug("function '%s' done", name.latin1());
    return new Operation(name, argsArr, args.count());
}

void
ExpressionParser::errorMessage(const char* msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    tokenizer.errorMessageVA(msg, ap);
    va_end(ap);
}
