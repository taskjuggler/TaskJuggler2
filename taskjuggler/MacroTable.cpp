/*
 * MacroTable.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
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
MacroTable::resolve()
{
    QPtrListIterator<QStringList> pli(argStack);
    pli.toLast();
    QString nameWithPrefix = (*(*pli))[0];
    if (DEBUGMA(10))
    {
        qDebug("MacroTable::resolve():");
        for (int i = 1; *pli; --pli, ++i)
        {
            QStringList* sl = *pli;
            if (sl)
            {
                qDebug(" Argument Stack Level %d", i);
                QStringList::Iterator it = sl->begin();
                for (int j = 1; it != sl->end(); ++it, ++j)
                    qDebug("  %d: %s", j, (*it).latin1());
            }
            else
                qDebug("Argument Stack Level %d empty", i);
        }
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
    if (isdigit(name[0].latin1()))
    {
        /* If the first character of the name is a digit, we assume that it is
         * a number. It addresses the n-th argument of the macro call. */
        QPtrListIterator<QStringList> pli(argStack);
        pli.toLast();
        /* The numerical arguments do not reference parameters of their own
         * call but of the caller. So we have to pop the stack once. */
        --pli;
        QStringList* sl = *pli;
        uint idx = name.toInt();
        if (sl == 0)
        {
            if (!emptyIsLegal)
                errorMessage(i18n("Macro argument stack is empty."));
            popArguments();
            return QString::null;
        }
        if (idx >= sl->count())
        {
            if (!emptyIsLegal)
                errorMessage
                    (i18n("Index %1 for argument out of range [0 - %2]!")
                     .arg(idx).arg(sl->count()));
            popArguments();
            return QString::null;
        }
        result = (*sl)[idx];
    }
    else
        if (name == "if")
        {
            QPtrListIterator<QStringList> pli(argStack);
            pli.toLast();
            QStringList* sl = *pli;
            if (!sl || sl->count() != 3)
            {
                errorMessage
                    (i18n("'if' macro needs a condition and an argument"));
                return QString::null;
            }
            if (!(*sl)[1].isEmpty() && (*sl)[1].toLong() != 0)
                result = (*sl)[2];
            else
                return QString::null;
        }
        else if (name == "ifelse")
        {
        }
        else if (name == "error")
        {
        }
        else if (name == "warning")
        {
        }
        else if (macros[name])
            result = expand(macros[name]->getValue());
            
    if (result.isNull() && !emptyIsLegal)
        errorMessage
            (i18n("Usage of undefined macro '%1'").arg(name));

    if (DEBUGMA(5))
        qDebug("Resolved as %s", result.latin1());

    popArguments();
    return result;
}

QString
MacroTable::expand(const QString& text)
{
    if (DEBUGMA(5))
        qDebug("MacroTable::expand(%s)", text.latin1());
    QString res;
    for (uint i = 0; i < text.length(); i++)
    {
        if (text[i] == '$')
        {
            if (i + 1 >= text.length() || text[i + 1] != '{')
            {
                res += '$';
                continue;
            }
            i += 2;
            // Skip white spaces
            while (i < text.length() && isspace(text[i]))
                ++i;
            if (i >= text.length())
            {
                errorMessage
                    (i18n("Macro call cannot be empty"));
                return res;
            }

            // Get macro call
            QString macroCall;
            while (i < text.length() && !isspace(text[i]) && text[i] != '}')
                macroCall += text[i++];
            if (i >= text.length())
            {
                errorMessage
                    (i18n("Unexpected end of macro call: %1").arg(text));
                return res;
            }
            QStringList* argumentList = new QStringList;
            argumentList->append(macroCall);

            // Skip white spaces
            while (i < text.length() && isspace(text[i]))
                ++i;
            if (i >= text.length())
            {
                errorMessage
                    (i18n("Unexpected end of macro: %1").arg(text));
                return res;
            }

            // Read optional macro arguments
            while (i < text.length() && text[i] != '}')
            {
                // Check and remember argument delimiter
                if (text[i] != '"' && text[i] != '\'')
                {
                    errorMessage
                        (i18n("Macro parameters must be enclosed with quotes "
                              "or double quotes."));
                    return res;
                }
                QChar delim = text[i++];
                QString arg;
                while (i < text.length() && text[i] != delim)
                    arg += text[i++];
                if (i >= text.length())
                {
                    errorMessage
                        (i18n("Unterminated macro argument: %1").arg(arg));
                    return res;
                }
                // Skip right delimiter
                i++;
                arg = expand(arg);
                argumentList->append(arg);
                
                // Skip white spaces
                while (isspace(text[i]))
                    ++i;
            }
            if (i >= text.length())
            {
                errorMessage
                    (i18n("Macro calls must be terminated with a '}': %1")
                     .arg(text));
                return res;
            }
            
            pushArguments(argumentList);
            res += resolve();
        }
        else
            res += text[i];
    }
    if (DEBUGMA(10))
        qDebug("Expanded %s to %s", text.latin1(), res.latin1());
    return res;
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


