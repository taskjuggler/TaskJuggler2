/*
 * Tokenizer.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef _Tokenizer_h_
#define _Tokenizer_h_

#include <qptrlist.h>

#include "MacroTable.h"
#include "Token.h"

class QString;
class ProjectFile;

/**
 * @short A fairly generic tokenizer class that breaks QTextStreams into small
 * pieces.
 * @author Chris Schlaeger <cs@suse.de>
 */
class Tokenizer
{
public:
    Tokenizer(const QString& file_, MacroTable* mt_, const QString& tp);
    Tokenizer(const QString& text);
    ~Tokenizer() { }

    TokenType nextToken(QString& buf);
    void returnToken(TokenType t, const QString& buf);

    bool open();
    bool close();

    const QString& getFile() const { return file; }
    QString getPath() const;

    const QString& getTaskPrefix() const { return taskPrefix; }

    int getLine() const { return currLine; }

    void errorMessageVA(const char* msg, va_list ap);
    void errorMessage(const char* msg, ...);

private:
    QChar getC(bool expandMacros = TRUE);
    void ungetC(QChar c);
    void expandMarco(QString& c);

    bool readMacroCall();

    bool getDateFragment(QString& token, QChar& c);
    bool readEnvironment();
    QString cleanupLine(const QString& line);

    /**
     * A pointer to the ProjectFile class that stores all read-in
     * data.
     */
    MacroTable* mt;

    // The name of the file.
    QString file;

    // The file handle of the file to read.
    FILE* fh;

    // The text buffer to read.
    QString textBuffer;

    // The stream used to read the file.
    QTextStream* f;

    // The number of the line currently being read.
    int currLine;

    /**
     * Macros have file scope. So we keep a stack of macros for each file that
     * we read.
     */
    QPtrList<Macro> macroStack;

    /**
     * A buffer for the part of the line that has been parsed already. This is
     * primarily used for error reporting.
     */
    QString lineBuf;

    /**
     * A buffer for characters that have been pushed back again. This
     * simplifies file parsing in some situations.
     */
    QValueList<QChar> ungetBuf;

    /**
     * Besides read in characters we can also push back a token. Contrary to
     * characters we can push back only 1 token. This is stored as type and
     * a string buffer.
     */
    TokenType tokenTypeBuf;
    QString tokenBuf;

    /**
     * Task trees of include files can not only be added at global scope but
     * also as sub-trees. This strings stores the prefix that has to be
     * specified at include times.
     */
    QString taskPrefix;
};

#endif

