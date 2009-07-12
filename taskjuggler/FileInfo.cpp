/*
 * FileInfo.cpp - TaskJuggler
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

#include "FileInfo.h"

#include <ctype.h>
#include <stdlib.h>

#include "TjMessageHandler.h"
#include "tjlib-internal.h"
#include "ProjectFile.h"
#include "debug.h"

FileInfo::FileInfo(ProjectFile* p, const QString& file, const QString& tp) :
    FileToken(file, tp),
    pf(p),
    oldLineBuf(),
    oldLine(0)
{
    localmacros.setAutoDelete(false);
}

bool
FileInfo::open()
{
    if (m_file.right(2) == "/.")
    {
        m_f.reset(new QTextStream(stdin, IO_ReadOnly));
        m_fh = stdin;
    }
    else
    {
        if ((m_fh = fopen(m_file, "r")) == 0)
            return false;
        m_f.reset(new QTextStream(m_fh, IO_ReadOnly));
    }

    if (DEBUGLEVEL > 0)
        tjWarning(i18n("Processing file \'%1\'").arg(m_file));

    m_lineBuf = oldLineBuf = QString::null;
    m_currLine = oldLine = 1;
    return true;
}

bool
FileInfo::close()
{
    for (QDictIterator<Macro> di(localmacros); *di; ++di)
    {
        pf->getMacros().deleteMacro((*di)->getName());
    }

    if (m_fh == stdin)
        return true;

    if (fclose(m_fh) == EOF)
        return false;

    return true;
}

QChar
FileInfo::getC(bool expandMacros)
{
 BEGIN:
    QChar c;
    if (m_ungetBuf.isEmpty())
    {
        if (feof(m_fh))
            c = QChar(EOFile);
        else
        {
            *m_f >> c;
            if (c == QChar('\r'))
            {
                if (!feof(m_fh))
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
    oldLineBuf = m_lineBuf;
    m_lineBuf += c;

    if (expandMacros)
    {
        if (c == '$')
        {
            QChar d;
            if ((d = getC(false)) == '{')
            {
                // remove ${ from m_lineBuf;
                oldLineBuf = m_lineBuf;
                m_lineBuf = m_lineBuf.left(m_lineBuf.length() - 2);
                readMacroCall();
                goto BEGIN;
            }
            else if (d == '(')
            {
                // remove $( from m_lineBuf;
                oldLineBuf = m_lineBuf;
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
FileInfo::ungetC(QChar c)
{
    oldLineBuf = m_lineBuf;
    m_lineBuf = m_lineBuf.left(m_lineBuf.length() - 1);
    m_ungetBuf.append(c);
}

TokenType
FileInfo::nextToken(QString& token)
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
                    if (c == '\n')
                    {
                        oldLine = m_currLine;
                        m_currLine++;
                    }
                    while ((c = getC(false)) != '*')
                    {
                        if (c == '\n')
                        {
                            oldLine = m_currLine;
                            m_currLine++;
                        }
                        else if (c.unicode() == EOFile)
                        {
                            errorMessage(i18n("Unterminated comment"));
                            return EndOfFile;
                        }
                    }
                } while ((c = getC(false)) != '/');
                break;
            }
            // This code skips C++-style comments like the one you are
            // reading here.
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
            {
                oldLine = m_currLine;
                m_currLine++;
            }
            oldLineBuf = m_lineBuf;
            m_lineBuf = "";
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
        else if (c == '\'' || c == '\"')
        {
            // single or double quoted string
            QChar delimiter = c;
            bool escape = false;
            while ((c = getC()).unicode() != EOFile &&
                   (escape || (c != delimiter)))
            {
                if ((c == '\n') && m_macroStack.isEmpty())
                {
                    oldLine = m_currLine;
                    m_currLine++;
                }
                if (c == '\\' && !escape)
                    escape = true;
                else
                {
                    escape = false;
                    token += c;
                }
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
                {
                    oldLine = m_currLine;
                    m_currLine++; // m_macroStack.isEmpty ??
                }
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
            case '*':
                return STAR;
            default:
                errorMessage(i18n("Illegal character '%1'").arg(c));
                return INVALID;
            }
        }
    }
}

void
FileInfo::errorMessage(const QString& msg)
{
    if (m_macroStack.isEmpty())
    {
        if (m_tokenTypeBuf == INVALID)
            TJMH.errorMessage(QString("%1\n%2").arg(msg)
                              .arg(cleanupLine(m_lineBuf)),
                              m_file, m_currLine);
        else
            TJMH.errorMessage(QString("%1\n%2").arg(msg)
                              .arg(cleanupLine(oldLineBuf)),
                              m_file, oldLine);
    }
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
                          arg(msg).arg(cleanupLine(m_lineBuf)).arg(stackDump),
                          file, line);
    }
}

void
FileInfo::warningMessage(const QString& msg)
{
    if (m_macroStack.isEmpty())
    {
        if (m_tokenTypeBuf == INVALID)
            TJMH.warningMessage(QString("%1\n%2").arg(msg)
                                .arg(cleanupLine(m_lineBuf)),
                                m_file, m_currLine);
        else
            TJMH.warningMessage(QString("%1\n%2").arg(msg)
                                .arg(cleanupLine(oldLineBuf)),
                                m_file, oldLine);
    }
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
        TJMH.warningMessage(i18n("Warning in expanded macro\n%1\n%2"
                                 "\nThis is the macro call stack:%3").
                            arg(msg).arg(cleanupLine(m_lineBuf)).arg(stackDump),
                            file, line);
    }
}

void FileInfo::setLocation(const QString& df, int dl)
{
    pf->getMacros().setLocation(df, dl);
}

QString FileInfo::resolve(const QStringList* argList)
{
    return pf->getMacros().resolve(argList);
}

Macro* FileInfo::getMacro(const QString& name) const
{
    return pf->getMacros().getMacro(name);
}
