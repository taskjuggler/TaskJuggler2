/*
 * FileInfo.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef _FileInfo_h_
#define _FileInfo_h_

#include <qptrlist.h>

#include "MacroTable.h"
#include "Token.h"

class QString;
class ProjectFile;

/**
 * @short Stores much information about a project file.
 * @author Chris Schlaeger <cs@kde.org>
 */
class FileInfo
{
public:
    FileInfo(ProjectFile* p, const QString& file_, const QString& tp) :
        pf(p),
        file(file_),
        fh(0),
        f(0),
        currLine(0),
        macroStack(),
        lineBuf(),
        ungetBuf(),
        tokenTypeBuf(INVALID),
        tokenBuf(),
        oldLineBuf(),
        oldLine(0),
        taskPrefix(tp)
    { }

    ~FileInfo()
    {
        delete f;
    }

    bool open();
    bool close();

    QChar getC(bool expandMacros = TRUE);
    void ungetC(QChar c);
    void expandMarco(QString& c);

    const QString& getFile() const { return file; }
    QString getPath() const;

    int getLine() const { return currLine; }

    TokenType nextToken(QString& buf);
    void returnToken(TokenType t, const QString& buf);

    bool readMacroCall();
    bool readEnvironment();

    const QString& getTaskPrefix() const { return taskPrefix; }

    void errorMessageVA(const char* msg, va_list ap);
    void errorMessage(const char* msg, ...);

private:
    bool getDateFragment(QString& token, QChar& c);
    QString cleanupLine(const QString& line);

    /**
     * A pointer to the ProjectFile class that stores all read-in
     * data.
     */
    ProjectFile* pf;

    // The name of the file.
    QString file;

    // The file handle of the file to read.
    FILE* fh;

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
     * In case of a returned token, we also have to save the current line
     * buffer and the current line number, in case an error has occured before
     * the pushed back token.
     */
    QString oldLineBuf;
    int oldLine;

    /**
     * Task trees of include files can not only be added at global scope but
     * also as sub-trees. This strings stores the prefix that has to be
     * specified at include times.
     */
    QString taskPrefix;
};

#endif

