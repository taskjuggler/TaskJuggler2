/*
 * MacroTable.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <ctype.h>
#include <stdio.h>
#include <qregexp.h>

#include "debug.h"
#include "config.h"
#include "TjMessageHandler.h"
#include "tjlib-internal.h"
#include "MacroTable.h"

bool
MacroTable::addMacro(Macro* macro)
{
    if (macros[macro->getName()])
        return FALSE;
    macros.insert(macro->getName(), macro);
    return TRUE;
}

void
MacroTable::setMacro(Macro* macro)
{
    macros.replace(macro->getName(), macro);
}

QString
MacroTable::resolve(const QStringList* argList)
{
    QString nameWithPrefix = (*argList)[0];
    if (DEBUGMA(10))
    {
        qDebug("MacroTable::resolve(%s):", (*argList)[0].latin1());
        for (uint i = 1; i < argList->count(); ++i)
            qDebug("  ${%d}: %s", i, (*argList)[i].latin1());
    }

    QString name = nameWithPrefix;
    bool emptyIsLegal = FALSE;

    if (name[0].latin1() == '?')
    {
        /* If the first character of the name is a question mark, the
         * macro may be undefined. */
        name = name.mid(1);
        if (name.isEmpty())
            errorMessage(i18n("'?' must be followed by a valid macro name!"));
        emptyIsLegal = TRUE;
    }

    QString result;
    if (name == "if")
    {
        if (!argList || argList->count() != 3)
        {
            errorMessage
                (i18n("'if' macro needs a condition and an argument"));
            return QString::null;
        }
        if (!(*argList)[1].isEmpty() && evalExpression((*argList)[1]))
            result = (*argList)[2];
        else
            return QString::null;
    }
    else if (name == "ifelse")
    {
        if (!argList || argList->count() != 4)
        {
            errorMessage
                (i18n("'ifelse' macro needs a condition and two arguments"));
            return QString::null;
        }
        if (!(*argList)[1].isEmpty() && evalExpression((*argList)[1]))
            result = (*argList)[2];
        else
            result = (*argList)[3];
    }
    else if (name == "error")
    {
        if (!argList || argList->count() != 2)
        {
            errorMessage
                (i18n("'error' macro needs one string argument"));
            return QString::null;
        }
        qWarning("%s", (*argList)[1].latin1());
        return QString::null; 
    }
    else if (name == "warning")
    {
        if (!argList || argList->count() != 2)
        {
            errorMessage
                (i18n("'warning' macro needs one string argument"));
            return QString::null;
        }
        qWarning("%s", (*argList)[1].latin1());
    }
    else if (name == "version")
    {
        int maj, min, pl;
        sscanf(VERSION, "%d.%d.%d", &maj, &min, &pl);
        QString res; res.sprintf("%2d%02d%02d", maj, min, pl);
        return res;
    }
    else if (macros[name])
    {
        QString resolved = macros[name]->getValue();
        if (DEBUGMA(15))
            qDebug("Found %s as [%s]", name.latin1(), resolved.latin1());

        /* Now replace the argument placeholders (e. g. ${1}, ${2} with the
         * real argument values. */
        for (uint i = 0; i < resolved.length(); )
        {
            if (resolved[i] == '$' && i + 3 < resolved.length() &&
                resolved[i + 1] == '{' &&
                isdigit(resolved[i + 2].latin1()))
            {
                i += 2;
                QString num;
                while (i < resolved.length() &&
                       isdigit(resolved[i].latin1()))
                    num += resolved[i++];
                if (num.isEmpty())
                {
                    errorMessage(i18n("Number expected: %s")
                                 .arg(resolved.left(i).latin1()));
                    return QString::null;
                }
                if (i >= resolved.length() || resolved[i++] != '}')
                {
                    errorMessage(i18n("'}' expected"));
                    return QString::null;
                }
                uint number = num.toUInt();
                if (number > argList->count())
                {
                    errorMessage(i18n("Macro %s only has ${%s} parameters")
                                 .arg(name).arg(num));
                    return QString::null;
                }
                result += (*argList)[number];
            }
            else
                result += resolved[i++];
        }
        if (DEBUGMA(15))
            qDebug("After argument expansion: [%s]", result.latin1());
    }
            
    if (result.isNull() && !emptyIsLegal)
        errorMessage
            (i18n("Usage of undefined macro '%1'").arg(name));

    if (DEBUGMA(5))
        qDebug("Resolved as %s", result.latin1());

    return result;
}

QString
MacroTable::expandReportVariable(QString text, const QStringList* argList)
{
    if (DEBUGMA(5))
        qDebug("MacroTable::expandReportVariable(%s)", text.latin1());

    QString res;
    for (uint i = 0; i < text.length();)
    {
        if (text[i] == '%')
        {
            if (i + 3 >= text.length() || text[i + 1] != '{')
            {
                res += '%';
                continue;
            }
            i += 2;
            // Get report variable name
            QString varName;
            while (i < text.length() && text[i] != '}')
                varName += text[i++];
            if (varName.isEmpty())
            {
                errorMessage
                    (i18n("Unexpected end of report variable: %1")
                     .arg(text.left(i)));
                return QString::null;
            }
            if (i >= text.length() || text[i++] != '}')
            {
                errorMessage
                    (i18n("Macro calls must be terminated with a '}': %1")
                     .arg(text.left(i)));
                return QString::null;
            }
            
            if (varName == "0")
                res += (*argList)[0];
            else 
            {
                QStringList sl;
                sl.append(varName);
                res += resolve(&sl);
            }
        }
        else
            res += text[i++];
    }
    if (DEBUGMA(10))
        qDebug("Expanded %s to %s", text.latin1(), res.latin1());
    return res;
}

bool
MacroTable::evalExpression(const QString expr) const
{
    // This is a very simple expression evaluator for static expressions.

    QString arg1, arg2, op;
   
    uint i = 0;
    QChar delim;
    // Skip spaces
    while (i < expr.length() && expr[i] == " ")
        i++;
    if (i >= expr.length())
       goto UNEXPEND;

    // Get first argument
    if ((delim = expr[i]) == '\'' || delim == '"')
        i++;
    else
        delim = ' ';
    while (i < expr.length() && expr[i] != delim)
        arg1 += expr[i++];
    if (i >= expr.length())
       goto UNEXPEND;

    // Skip spaces
    while (i < expr.length() && expr[i] == " ")
        i++;
    if (i >= expr.length())
       goto UNEXPEND;

    // Get operator
    while (i < expr.length() && expr[i] != " ")
        op += expr[i++];

    // Skip spaces
    while (i < expr.length() && expr[i] == " ")
        i++;
    if (i >= expr.length())
       goto UNEXPEND;

    // Get second argument
    if ((delim = expr[i]) == '\'' || delim == '"')
        i++;
    else
        delim = ' ';
    while (i < expr.length() && expr[i] != delim)
        arg2 += expr[i++];

    // Skip spaces
    while (i < expr.length() && expr[i] == " ")
        i++;

    // Now we must have reached the end of the expression
    if (i < expr.length())
    {
        errorMessage
            (i18n("Garbage at end of expression"));
        return FALSE;
    }

    if (op == "=")
    {
        return arg1 == arg2;
    }
    else if (op == "!=")
    {
        return arg1 != arg2;
    }
    else if (op == "<")
    {
        return arg1.toLong() < arg2.toLong();
    }
    else if (op == ">")
    {
        return arg1.toLong() > arg2.toLong();
    } 
    else if (op == "<=")
    {
        return arg1.toLong() <= arg2.toLong();
    }
    else if (op == ">=")
    {
        return arg1.toLong() >= arg2.toLong();
    }
    else
    {
       errorMessage(i18n("Illegal operator: %1").arg(op.latin1()));
       return FALSE;
    }

UNEXPEND:
    errorMessage(i18n("Unexpected end of expression: %1").arg(expr.left(i)));
    return FALSE; 
}

void
MacroTable::errorMessage(const char* msg, ... ) const
{
    va_list ap;
    va_start(ap, msg);
    char buf[1024];
    vsnprintf(buf, 1024, msg, ap);
    va_end(ap);

    TJMH.errorMessage(buf, defFileName, defFileLine);
}


