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

#include "FileToken.h"

class ProjectFile;

/**
 * @short Stores much information about a project file.
 * @author Chris Schlaeger <cs@kde.org>
 */
class FileInfo : public FileToken
{
public:
    FileInfo(ProjectFile* p, const QString& file, const QString& tp);

    virtual ~FileInfo() { }

    bool open();
    bool close();

    virtual QChar getC(bool expandMacros = true);
    void ungetC(QChar c);

    virtual TokenType nextToken(QString& buf);

    virtual void setLocation(const QString& df, int dl);
    virtual QString resolve(const QStringList* argList);
    virtual Macro* getMacro(const QString& name) const;

    virtual void errorMessage(const QString& msg);
    virtual void warningMessage(const QString& msg);

private:
    /**
     * A pointer to the ProjectFile class that stores all read-in
     * data.
     */
    ProjectFile* pf;

    /**
     * In case of a returned token, we also have to save the current line
     * buffer and the current line number, in case an error has occured before
     * the pushed back token.
     */
    QString oldLineBuf;
    int oldLine;
};

#endif

