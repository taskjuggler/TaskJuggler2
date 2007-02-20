/*
 * Tokenizer.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006
 * Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "Tokenizer.h"

#include <ctype.h>
#include <stdlib.h>

#include "TjMessageHandler.h"
#include "tjlib-internal.h"
#include "ProjectFile.h"
#include "debug.h"

Tokenizer::Tokenizer(const QString& file_, MacroTable* mt_, const QString& tp) :
    mt(mt_),
    file(file_),
    fh(0),
    textBuffer(QString::null),
    f(0),
    currLine(0),
    macroStack(),
    lineBuf(),
    ungetBuf(),
    tokenTypeBuf(INVALID),
    tokenBuf(),
    taskPrefix(tp)
{
}

Tokenizer::Tokenizer(const QString& buf) :
    mt(0),
    file(),
    fh(0),
    textBuffer(buf),
    f(0),
    currLine(0),
    macroStack(),
    lineBuf(),
    ungetBuf(),
    tokenTypeBuf(INVALID),
    tokenBuf(),
    taskPrefix()
{
}

TokenType
Tokenizer::nextToken(QString& token)
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

    // analyze non blank characters
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
                if ((c == '\n') && macroStack.isEmpty())
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
                if ((c == '\n') && macroStack.isEmpty())
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
            token = "";
            int nesting = 0;
            while ((c = getC(FALSE)).unicode() != EOFile &&
                   (c != ']' || nesting > 0))
            {
                if (c == '[')
                    nesting++;
                else if (c == ']')
                    nesting--;
                if (c == '\n')
                    currLine++;			// macroStack.isEmpty ??
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
                return LBRACE;
            case '}':
                return RBRACE;
            case '(':
                return LBRACKET;
            case ')':
                return RBRACKET;
            case ',':
                return COMMA;
            case '%':
                return PERCENT;
            case '~':
                return TILDE;
            case ':':
                return COLON;
            case '?':
                return QUESTIONMARK;
            case '+':
                return PLUS;
            case '-':
                return MINUS;
            case '&':
                return AND;
            case '|':
                return OR;
            case '>':
            {
                if ((c = getC()) == '=')
                {
                    token += c;
                    return GREATEROREQUAL;
                }
                ungetC(c);
                return GREATER;
            }
            case '<':
            {
                if ((c = getC()) == '=')
                {
                    token += c;
                    return SMALLEROREQUAL;
                }
                ungetC(c);
                return SMALLER;
            }
            case '=':
                return EQUAL;
            default:
                errorMessage(i18n("Illegal character '%1'").arg(c));
                return INVALID;
            }
        }
    }
}

void
Tokenizer::returnToken(TokenType tt, const QString& buf)
{
    if (tokenTypeBuf != INVALID)
    {
        qFatal("Internal Error: Token buffer overflow!");
        return;
    }
    tokenTypeBuf = tt;
    tokenBuf = buf;
}

bool
Tokenizer::open()
{
    if (!file.isEmpty())
    {
        /* The calling functions always prepend the name of the parent file or
         * the current directory of the application. We use '.' as name for
         * reading stdin. So in this case the last 2 characters of the file
         * name are "/.". */
        if (file.right(2) == "/.")
        {
            // read from stdin
            f = new QTextStream(stdin, IO_ReadOnly);
            fh = stdin;
        }
        else
        {
            // read from file system
            if ((fh = fopen(file, "r")) == 0)
                return FALSE;
            f = new QTextStream(fh, IO_ReadOnly);
        }

        if (DEBUGLEVEL > 0)
            qWarning(i18n("Processing file \'%1\'").arg(file));
    }
    else
        f = new QTextStream(textBuffer, IO_ReadOnly);

    lineBuf = QString::null;
    currLine = 1;

    return TRUE;
}

bool
Tokenizer::close()
{
    if (!file.isEmpty())
    {
        if (fh == stdin)
            return TRUE;

        if (fclose(fh) == EOF)
            return FALSE;
    }
    else
        delete f;

    return TRUE;
}

QChar
Tokenizer::getC(bool expandMacros)
{
 BEGIN:
    QChar c;
    if (ungetBuf.isEmpty())
    {
        if (f->atEnd())
            c = QChar(EOFile);
        else
        {
            *f >> c;
            if (c == QChar('\r'))
            {
                if (!f->atEnd())
                {
                    // Test for CR/LF Windows line breaks.
                    QChar cb;
                    *f >> cb;
                    if (cb != QChar('\n'))
                    {
                        // Probably a MacOS LF only line break
                        ungetBuf.append(cb);
                    }
                }
                c = QChar('\n');
            }
        }
    }
    else
    {
        c = ungetBuf.last();
        ungetBuf.pop_back();
        if (c.unicode() == EOMacro)
        {
            macroStack.removeLast();
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
                // remove ${ from lineBuf;
                lineBuf = lineBuf.left(lineBuf.length() - 2);
                readMacroCall();
                goto BEGIN;
            }
            else if (d == '(')
            {
                // remove $( from lineBuf;
                lineBuf = lineBuf.left(lineBuf.length() - 2);
                readEnvironment();
                goto BEGIN;
            }
            else if (d == '$')
            {
                QChar e;
                if ((e = getC(FALSE)) == '{')
                {
                    // Convert "$${" into "%{"
                    c = '%';
                }
                // $$ escapes $, so discard 2nd $
                ungetC(e);
            }
            else
                ungetC(d);
        }
    }

    return c;
}

void
Tokenizer::ungetC(QChar c)
{
    lineBuf = lineBuf.left(lineBuf.length() - 1);
    ungetBuf.append(c);
}

bool
Tokenizer::getDateFragment(QString& token, QChar& c)
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
Tokenizer::getPath() const
{
    if (file.find('/') >= 0)
        return file.left(file.findRev('/') + 1);
    else
        return "";
}

bool
Tokenizer::readMacroCall()
{
    QString id;
    TokenType tt;
    /* For error reporting we need to replace the macro call with the macro
     * text. So we save a copy of the current line buf (the ${ has already
     * been removed) and copy it over the lineBuf again after we have read the
     * complete macro call. */
    QString lineBufCopy = lineBuf;
    QString prefix;
    if ((tt = nextToken(id)) == QUESTIONMARK)
    {
        prefix = "?";
    }
    else
        returnToken(tt, id);

    if ((tt = nextToken(id)) != ID && tt != INTEGER)
    {
        errorMessage(i18n("Macro ID expected"));
        return FALSE;
    }
    id = prefix + id;

    QString token;
    // Store all arguments in a newly created string list.
    QStringList* sl = new QStringList;
    sl->append(id);
    while ((tt = nextToken(token)) == STRING || tt == ID)
        sl->append(token);
    if (tt != RBRACE)
    {
        errorMessage(i18n("'}' expected"));
        delete sl;
        return FALSE;
    }

    // expand the macro
    mt->setLocation(file, currLine);
    QString macro = mt->resolve(sl);

    if (macro.isNull() && prefix.isEmpty())
    {
        delete sl;
        return FALSE;
    }

    lineBuf = lineBufCopy;

    // Push pointer to macro on stack. Needed for error handling.
    macroStack.append(mt->getMacro(id));

    // mark end of macro
    ungetBuf.append(QChar(EOMacro));
    // push expanded macro reverse into ungetC buffer.
    for (int i = macro.length() - 1; i >= 0; --i)
        ungetBuf.append(macro[i].latin1());

    delete sl;
    return TRUE;
}

bool
Tokenizer::readEnvironment()
{
    QString id;

    if (nextToken(id) != ID)
    {
        errorMessage(i18n("Environment name expected"));
        return FALSE;
    }

    QString token;
    if (nextToken(token) != RBRACKET)
    {
        errorMessage(i18n("')' expected"));
        return FALSE;
    }

    char *value = getenv (id.ascii());

    if (value != 0)
        id = value;
    else
        id = "";

    // push expanded macro reverse into ungetC buffer.
    for (int i = id.length() - 1; i >= 0; --i)
        ungetBuf.append(id[i].latin1());

    return TRUE;
}

QString
Tokenizer::cleanupLine(const QString& line)
{
    QString res;
    for (uint i = 0; i < line.length(); ++i)
        if (line[i] != QChar(EOMacro))
            res += line[i];

    return res;
}

void
Tokenizer::errorMessage(const char* msg, ...)
{
    va_list ap;
    char buf[1024];
    va_start(ap, msg);
    vsnprintf(buf, 1024, msg, ap);
    va_end(ap);

    if (macroStack.isEmpty())
        TJMH.errorMessage(QString("%1\n%2").arg(buf).arg(cleanupLine(lineBuf)),
                          file, currLine);
    else
    {
        QString stackDump;
        int i = 0;
        QString file;
        int line = 0;
        for (QPtrListIterator<Macro> mli(macroStack); *mli; ++mli, ++i)
        {
            stackDump += "\n  ${" + (*mli)->getName() + " ... }";

            file = (*mli)->getFile();
            line = (*mli)->getLine();
        }
        TJMH.errorMessage(i18n("Error in expanded macro\n%1\n%2"
                               "\nThis is the macro call stack:%3").
                          arg(buf).arg(cleanupLine(lineBuf)).arg(stackDump),
                          file, line);
    }
}

void
Tokenizer::errorMessageVA(const char* msg, va_list ap)
{
    char buf[1024];
    vsnprintf(buf, 1024, msg, ap);

    errorMessage("%s", buf);
}

