/*
 * FileToken.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id: $
 */
#include "FileToken.h"

#include <stdlib.h>

#include "tjlib-internal.h"

FileToken::FileToken(const QString& file, const QString& tp) :
    m_file(file),
    m_fh(0),
    m_f(0),
    m_currLine(0),
    m_macroStack(),
    m_lineBuf(),
    m_ungetBuf(),
    m_tokenTypeBuf(INVALID),
    m_tokenBuf(),
    m_taskPrefix(tp)
{ }

FileToken::FileToken() :
    m_file(),
    m_fh(0),
    m_f(0),
    m_currLine(0),
    m_macroStack(),
    m_lineBuf(),
    m_ungetBuf(),
    m_tokenTypeBuf(INVALID),
    m_tokenBuf(),
    m_taskPrefix()
{ }

QString FileToken::getPath() const
{
    if (m_file.find('/') >= 0)
        return m_file.left(m_file.findRev('/') + 1);
    else
        return "";
}

void FileToken::returnToken(TokenType tt, const QString& buf)
{
    if (m_tokenTypeBuf != INVALID)
    {
        qFatal("Internal Error: Token buffer overflow!");
        return;
    }
    m_tokenTypeBuf = tt;
    m_tokenBuf = buf;
}

QString FileToken::cleanupLine(const QString& line)
{
    QString res;
    for (uint i = 0; i < line.length(); ++i)
        if (line[i] != QChar(EOMacro))
            res += line[i];

    return res;
}

bool FileToken::readEnvironment()
{
    QString id;

    if (nextToken(id) != ID)
    {
        errorMessage(i18n("Environment name expected"));
        return false;
    }

    QString token;
    if (nextToken(token) != RBRACKET)
    {
        errorMessage(i18n("')' expected"));
        return false;
    }

    char *value = getenv (id.ascii());
    id = ( value ? value : "" );

    // push expanded macro reverse into ungetC buffer.
    for (int i = id.length() - 1; i >= 0; --i)
        m_ungetBuf.append(id[i].latin1());

    return true;
}

bool FileToken::getDateFragment(QString& token, QChar& c)
{
    token += c;
    c = getC();
    // c must be a digit
    if (!c.isDigit())
    {
        errorMessage(i18n("Corrupted date"));
        return false;
    }
    token += c;
    // read other digits
    while ((c = getC()).unicode() != EOFile && c.isDigit())
        token += c;

    return true;
}

bool FileToken::readMacroCall()
{
    QString id;
    TokenType tt;
    /* For error reporting we need to replace the macro call with the macro
     * text. So we save a copy of the current line buf (the ${ has already
     * been removed) and copy it over the lineBuf again after we have read the
     * complete macro call. */
    QString lineBufCopy = m_lineBuf;
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
        return false;
    }
    id = prefix + id;

    QString token;
    // Store all arguments in a newly created string list.
    QStringList sl(id);
    while ((tt = nextToken(token)) == STRING || tt == ID)
        sl.append(token);
    if (tt != RBRACE)
    {
        errorMessage(i18n("'}' expected"));
        return false;
    }

    // expand the macro
    setLocation(m_file, m_currLine);
    QString macro = resolve(&sl);

    if (macro.isNull() && prefix.isEmpty())
        return false;

    m_lineBuf = lineBufCopy;

    if (m_macroStack.count() > 30)
    {
        errorMessage(i18n("Too many nested macro calls."));
        return false;
    }

    // Push pointer to macro on stack. Needed for error handling.
    m_macroStack.append(getMacro(id));

    // mark end of macro
    m_ungetBuf.append(QChar(EOMacro));
    // push expanded macro reverse into ungetC buffer.
    for (int i = macro.length() - 1; i >= 0; --i)
        m_ungetBuf.append(macro[i]);
    return true;
}
