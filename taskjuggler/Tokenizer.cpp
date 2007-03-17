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

Tokenizer::Tokenizer(const QString& file, MacroTable* mt_, const QString& tp) :
    FileToken(file, tp),
    mt(mt_),
    textBuffer(QString::null)
{
}
#if 0
    FileToken(file, tp),
    m_mt(mt),
    m_textBuffer(QString::null)
{ }
#endif

Tokenizer::Tokenizer(const QString& buf) :
    FileToken(),
    mt(0),
    textBuffer(buf)
{
}

TokenType
Tokenizer::nextToken(QString& token)
{
    if (m_tokenTypeBuf != INVALID)
    {
        token = m_tokenBuf;
        TokenType tt = m_tokenTypeBuf;
        m_tokenTypeBuf = INVALID;
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
            if ((c = getC(false)) == '*')
            {
                do
                {
                    while ((c = getC(false)) != '*')
                    {
                        if (c == '\n')
                            m_currLine++;
                        else if (c.unicode() == EOFile)
                        {
                            errorMessage(i18n("Unterminated comment"));
                            return EndOfFile;
                        }
                    }
                } while ((c = getC(false)) != '/');
                break;
            }
            // This code skips C++-style comments like the one you are reading
            // here.
            else if (c != '/')
            {
                ungetC(c);
                ungetC('/');
                goto BLANKS_DONE;
            }
            // break missing on purpose
        case '#':   // Comments start with '#' and reach towards end of line
            while ((c = getC(false)) != '\n' && c.unicode() != EOFile)
                ;
            if (c.unicode() == EOFile)
                return EndOfFile;
            // break missing on purpose
        case '\n':
            // Increase line counter only when not replaying a macro.
            if (m_macroStack.isEmpty())
                m_currLine++;
            m_lineBuf = "";
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
                if ((c == '\n') && m_macroStack.isEmpty())
                    m_currLine++;
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
                if ((c == '\n') && m_macroStack.isEmpty())
                    m_currLine++;
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
            while ((c = getC(false)).unicode() != EOFile &&
                   (c != ']' || nesting > 0))
            {
                if (c == '[')
                    nesting++;
                else if (c == ']')
                    nesting--;
                if (c == '\n')
                    m_currLine++; // m_macroStack.isEmpty ??
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

bool
Tokenizer::open()
{
    if (!m_file.isEmpty())
    {
        /* The calling functions always prepend the name of the parent file or
         * the current directory of the application. We use '.' as name for
         * reading stdin. So in this case the last 2 characters of the file
         * name are "/.". */
        if (m_file.right(2) == "/.")
        {
            // read from stdin
            m_f.reset(new QTextStream(stdin, IO_ReadOnly));
            m_fh = stdin;
        }
        else
        {
            // read from file system
            if ((m_fh = fopen(m_file, "r")) == 0)
                return false;
            m_f.reset(new QTextStream(m_fh, IO_ReadOnly));
        }

        if (DEBUGLEVEL > 0)
            qWarning(i18n("Processing file \'%1\'").arg(m_file));
    }
    else
        m_f.reset(new QTextStream(textBuffer, IO_ReadOnly));

    m_lineBuf = QString::null;
    m_currLine = 1;

    return true;
}

bool
Tokenizer::close()
{
    if (!m_file.isEmpty())
    {
        if (m_fh == stdin)
            return true;

        if (fclose(m_fh) == EOF)
            return false;
    }

    return true;
}

QChar
Tokenizer::getC(bool expandMacros)
{
 BEGIN:
    QChar c;
    if (m_ungetBuf.isEmpty())
    {
        if (m_f->atEnd())
            c = QChar(EOFile);
        else
        {
            *m_f >> c;
            if (c == QChar('\r'))
            {
                if (!m_f->atEnd())
                {
                    // Test for CR/LF Windows line breaks.
                    QChar cb;
                    *m_f >> cb;
                    if (cb != QChar('\n'))
                    {
                        // Probably a MacOS LF only line break
                        m_ungetBuf.append(cb);
                    }
                }
                c = QChar('\n');
            }
        }
    }
    else
    {
        c = m_ungetBuf.last();
        m_ungetBuf.pop_back();
        if (c.unicode() == EOMacro)
        {
            m_macroStack.removeLast();
            goto BEGIN;
        }
    }
    m_lineBuf += c;

    if (expandMacros)
    {
        if (c == '$')
        {
            QChar d;
            if ((d = getC(false)) == '{')
            {
                // remove ${ from m_lineBuf;
                m_lineBuf = m_lineBuf.left(m_lineBuf.length() - 2);
                readMacroCall();
                goto BEGIN;
            }
            else if (d == '(')
            {
                // remove $( from m_lineBuf;
                m_lineBuf = m_lineBuf.left(m_lineBuf.length() - 2);
                readEnvironment();
                goto BEGIN;
            }
            else if (d == '$')
            {
                QChar e;
                if ((e = getC(false)) == '{')
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
    m_lineBuf = m_lineBuf.left(m_lineBuf.length() - 1);
    m_ungetBuf.append(c);
}

void
Tokenizer::errorMessage(const char* msg, ...)
{
    va_list ap;
    char buf[1024];
    va_start(ap, msg);
    vsnprintf(buf, 1024, msg, ap);
    va_end(ap);

    if (m_macroStack.isEmpty())
        TJMH.errorMessage(QString("%1\n%2").arg(buf).arg(cleanupLine(m_lineBuf)),
                          m_file, m_currLine);
    else
    {
        QString stackDump;
        int i = 0;
        QString file;
        int line = 0;
        for (QPtrListIterator<Macro> mli(m_macroStack); *mli; ++mli, ++i)
        {
            stackDump += "\n  ${" + (*mli)->getName() + " ... }";

            file = (*mli)->getFile();
            line = (*mli)->getLine();
        }
        TJMH.errorMessage(i18n("Error in expanded macro\n%1\n%2"
                               "\nThis is the macro call stack:%3").
                          arg(buf).arg(cleanupLine(m_lineBuf)).arg(stackDump),
                          file, line);
    }
}

void Tokenizer::setLocation(const QString& df, int dl)
{
    mt->setLocation(df, dl);
}

QString Tokenizer::resolve(const QStringList* argList)
{
    return mt->resolve(argList);
}

Macro* Tokenizer::getMacro(const QString& name) const
{
    return mt->getMacro(name);
}
