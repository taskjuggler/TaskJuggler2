/*
 * FileInfo.cpp - TaskJuggler
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

#include "TjMessageHandler.h"
#include "tjlib-internal.h"
#include "FileInfo.h"
#include "ProjectFile.h"

FileInfo::FileInfo(ProjectFile* p, const QString& file_, const QString& tp)
    : pf(p), taskPrefix(tp)
{
    tokenTypeBuf = INVALID;
    file = file_;
}

bool
FileInfo::open()
{
    if (file.right(2) == "/.")
    {
        f = new QTextStream(stdin, IO_ReadOnly);
        fh = stdin;
    }
    else
    {
        if ((fh = fopen(file, "r")) == 0)
            return FALSE;
        f = new QTextStream(fh, IO_ReadOnly);
    }

    lineBuf = QString::null;
    currLine = 1;
    return TRUE;
}

bool
FileInfo::close()
{
    delete f;
    if (fh == stdin)
        return TRUE;

    if (fclose(fh) == EOF)
        return FALSE;

    return TRUE;
}

QChar
FileInfo::getC(bool expandMacros)
{
 BEGIN:
    QChar c;
    if (ungetBuf.isEmpty())
    {
        *f >> c;
        if (feof(fh))
            c = QChar(EOFile);
    }
    else
    {
        c = ungetBuf.last();
        ungetBuf.remove(ungetBuf.fromLast());
        if (c.unicode() == EOMacro)
        {
            macroStack.removeLast();
            pf->getMacros().popArguments();
            goto BEGIN;
        }
    }
    lineBuf += c;

    if (expandMacros)
    {
        if (c == '$')
        {
            QChar d;
            if ((d = getC(FALSE)) == '{')
            {
                // remove $ from lineBuf;
                lineBuf = lineBuf.left(lineBuf.length() - 1);
                readMacroCall();
                goto BEGIN;
            }
            else
            {
                // $$ escapes $, so discard 2nd $
                if (d != '$')
                    ungetC(d);
            }
        }
    }

    return c;
}

void
FileInfo::ungetC(QChar c)
{
    lineBuf = lineBuf.left(lineBuf.length() - 1);
    ungetBuf.append(c);
}

bool
FileInfo::getDateFragment(QString& token, QChar& c)
{
    token += c;
    c = getC();
    // c must be a digit
    if (!c.isDigit())
    {
        errorMessage(i18n("Corrupted date"));
        return FALSE;
    }
    token += c;
    // read other digits
    while ((c = getC()).unicode() != EOFile && c.isDigit())
        token += c;

    return TRUE;
}

QString
FileInfo::getPath() const
{
    if (file.find('/') >= 0)
        return file.left(file.findRev('/') + 1);
    else
        return "";
}

TokenType
FileInfo::nextToken(QString& token)
{
    if (tokenTypeBuf != INVALID)
    {
        token = tokenBuf;
        TokenType tt = tokenTypeBuf;
        tokenTypeBuf = INVALID;
        return tt;
    }

    token = "";

    // skip blanks and comments
    for ( ; ; )
    {
        QChar c = getC();
        if (c.unicode() == EOFile)
            return EndOfFile;
        switch (c)
        {
        case ' ':
        case '\t':
            break;
        case '/':
            /* This code skips c-style comments like the one you are just
             * reading. */
            if ((c = getC(FALSE)) == '*')
            {
                do
                {
                    while ((c = getC(FALSE)) != '*')
                    {
                        if (c == '\n')
                            currLine++;
                        else if (c.unicode() == EOFile)
                        {
                            errorMessage(i18n("Unterminated comment"));
                            return EndOfFile;
                        }
                    }
                } while ((c = getC(FALSE)) != '/');
            }
            else
            {
                ungetC(c);
                ungetC('/');
                goto BLANKS_DONE;
            }
            break;
        case '#':   // Comments start with '#' and reach towards end of line
            while ((c = getC(FALSE)) != '\n' && c.unicode() != EOFile)
                ;
            if (c.unicode() == EOFile)
                return EndOfFile;
            // break missing on purpose
        case '\n':
            // Increase line counter only when not replaying a macro.
            if (macroStack.isEmpty())
                currLine++;
            lineBuf = "";
            break;
        default:
            ungetC(c);
            goto BLANKS_DONE;
        }
    }
 BLANKS_DONE:

    // analyse non blank characters
    for ( ; ; )
    {
        QChar c = getC();
        if (c.unicode() == EOFile)
        {
            errorMessage(i18n("Unexpected end of file"));
            return EndOfFile;
        }
        else if (isalpha(c) || (c == '_') || (c == '!'))
        {
            token += c;
            while ((c = getC()).unicode() != EOFile &&
                   (isalnum(c) || (c == '_') || (c == '.') || (c == '!')))
                token += c;
            ungetC(c);
            if (token[0] == '!')
                return RELATIVE_ID;
            if (token.contains('.'))
                return ABSOLUTE_ID;
            else
                return ID;
        }
        else if (c.isDigit())
        {
            // read first number (maybe a year)
            token += c;
            while ((c = getC()).unicode() != EOFile && c.isDigit())
                token += c;
            if (c == '-')
            {
                // this must be a ISO date yyyy-mm-dd[[-hh:mm:[ss]]-TZ]
                getDateFragment(token, c);
                if (c != '-')
                {
                    errorMessage(i18n("Corrupted date"));
                    return EndOfFile;
                }
                getDateFragment(token, c);
                if (c == '-')
                {
                    getDateFragment(token, c);
                    if (c != ':')
                    {
                        errorMessage(i18n("Corrupted date"));
                        return EndOfFile;
                    }
                    getDateFragment(token, c);
                    if (c == ':')
                        getDateFragment(token, c);
                }
                int i = 0;
                if (c == '-')
                {
                    /* Timezone can either be a name (ref.
                     * Utility::timezone2tz) or GMT[+-]hh:mm */
                    token += c;
                    while ((c = getC()).unicode() != EOFile &&
                           (isalnum(c) || c == '+' || c == '-' || c == ':')
                           && i++ < 9)
                        token += c;
                }
                ungetC(c);
                return DATE;
            }
            else if (c == '.')
            {
                // must be a real number
                token += c;
                while ((c = getC()).unicode() != EOFile && c.isDigit())
                    token += c;
                ungetC(c);
                return REAL;
            }
            else if (c == ':')
            {
                // must be a time (HH:MM)
                token += c;
                for (int i = 0; i < 2; i++)
                {
                    if ((c = getC()).unicode() != EOFile && c.isDigit())
                        token += c;
                    else
                    {
                        errorMessage(i18n("2 digits minutes expected"));
                        return EndOfFile;
                    }
                }
                return HOUR;
            }
            else
            {
                ungetC(c);
                return INTEGER;
            }
        }
        else if (c == '\'')
        {
            // single quoted string
            while ((c = getC()).unicode() != EOFile && c != '\'')
            {
                if (c == '\n')
                    currLine++;
                token += c;
            }
            if (c.unicode() == EOFile)
            {
                errorMessage(i18n("Non terminated string"));
                return EndOfFile;
            }
            return STRING;
        }
        else if (c == '"')
        {
            // double quoted string
            while ((c = getC()).unicode() != EOFile && c != '"')
            {
                if (c == '\n')
                    currLine++;
                token += c;
            }
            if (c.unicode() == EOFile)
            {
                errorMessage(i18n("Non terminated string"));
                return EndOfFile;
            }
            return STRING;
        }
        else if (c == '[')
        {
            int nesting = 0;
            while ((c = getC(FALSE)).unicode() != EOFile &&
                   (c != ']' || nesting > 0))
            {
                if (c == '[')
                    nesting++;
                else if (c == ']')
                    nesting--;
                if (c == '\n')
                    currLine++;
                token += c;
            }
            if (c.unicode() == EOFile)
            {
                errorMessage(i18n("Non terminated macro definition"));
                return EndOfFile;
            }
            return MacroBody;
        }
        else
        {
            token += c;
            switch (c)
            {
            case '{':
                return LCBRACE;
            case '}':
                return RCBRACE;
            case '(':
                return LBRACE;
            case ')':
                return RBRACE;
            case ',':
                return COMMA;
            case '~':
                return TILDE;
            case '-':
                return MINUS;
            case '&':
                return AND;
            case '|':
                return OR;
            default:
                errorMessage(i18n("Illegal character '%1'").arg(c));
                return INVALID;
            }
        }
    }
}

bool
FileInfo::readMacroCall()
{
    QString id;
    TokenType tt;
    if ((tt = nextToken(id)) != ID && tt != INTEGER)
    {
        errorMessage(i18n("Macro ID expected"));
        return FALSE;
    }
    QString token;
    // Store all arguments in a newly created string list.
    QStringList* sl = new QStringList;
    while ((tt = nextToken(token)) == STRING)
        sl->append(token);
    if (tt != RCBRACE)
    {
        errorMessage(i18n("'}' expected"));
        return FALSE;
    }

    // push string list to global argument stack
    pf->getMacros().pushArguments(sl);

    // expand the macro
    pf->getMacros().setLocation(file, currLine);
    QString macro = pf->getMacros().resolve(id);
    if (macro.isNull())
    {
        errorMessage(i18n(QString("Unknown macro ") + id));
        return FALSE;
    }

    // Push pointer to macro on stack. Needed for error handling.
    macroStack.append(pf->getMacros().getMacro(id));

    // mark end of macro
    ungetC(QChar(EOMacro));
    // push expanded macro reverse into ungetC buffer.
    for (int i = macro.length() - 1; i >= 0; --i)
        ungetC(macro[i].latin1());
    return TRUE;
}

void
FileInfo::returnToken(TokenType tt, const QString& buf)
{
    if (tokenTypeBuf != INVALID)
    {
        qFatal("Internal Error: Token buffer overflow!");
        return;
    }
    tokenTypeBuf = tt;
    tokenBuf = buf;
}

void
FileInfo::errorMessage(const char* msg, ...)
{
    va_list ap;
    char buf[1024];
    va_start(ap, msg);
    vsnprintf(buf, 1024, msg, ap);
    va_end(ap);
    
    if (macroStack.isEmpty())
        TJMH.errorMessage(QString("%1\n%2").arg(buf).arg(lineBuf),
                          file, currLine); 
    else
    {
        QString stackDump;
        int i = 0;
        for (QPtrListIterator<Macro> mli(macroStack); *mli; ++mli, ++i)
            stackDump += "\n  ${" + (*mli)->getName() + " \""
                + pf->getMacros().getArguments(i)->join("\" \"") + "\"}";
        TJMH.errorMessage(i18n("Error in expanded macro\n%1\n%2"
                               "\nThis is the macro call stack:%2").
                          arg(buf).arg(lineBuf).arg(stackDump),
                          macroStack.last()->getFile(),
                          macroStack.last()->getLine());
    }
}

void
FileInfo::errorMessageVA(const char* msg, va_list ap)
{
    char buf[1024];
    vsnprintf(buf, 1024, msg, ap);
    
    errorMessage("%s", buf);
}


