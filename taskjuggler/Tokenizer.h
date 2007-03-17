/*
 * Tokenizer.h - TaskJuggler
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
#ifndef _Tokenizer_h_
#define _Tokenizer_h_

#include "FileToken.h"

class ProjectFile;

/**
 * @short A fairly generic tokenizer class that breaks QTextStreams into small
 * pieces.
 * @author Chris Schlaeger <cs@kde.org>
 */
class Tokenizer : public FileToken
{
public:
    Tokenizer(const QString& file, MacroTable* mt_, const QString& tp);
    Tokenizer(const QString& text);
    virtual ~Tokenizer() { }

    bool open();
    bool close();

    virtual QChar getC(bool expandMacros = true);
    void ungetC(QChar c);

    virtual TokenType nextToken(QString& buf);

    virtual void setLocation(const QString& df, int dl);
    virtual QString resolve(const QStringList* argList);
    virtual Macro* getMacro(const QString& name) const;

    virtual void errorMessage(const char* msg, ...);

private:
    /**
     * A pointer to the ProjectFile class that stores all read-in
     * data.
     */
    MacroTable* mt;

    // The text buffer to read.
    QString textBuffer;
};

#endif

