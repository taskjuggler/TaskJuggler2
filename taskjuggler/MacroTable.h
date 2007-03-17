/*
 * MacroTable.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _MacroTable_h_
#define _MacroTable_h_

#include <stdarg.h>

#include <qstring.h>
#include <qstringlist.h>
#include <qdict.h>

class Macro
{
public:
    Macro(const QString& n, const QString& v, const QString& f, uint l)
        : name(n), value(v), file(f), line(l) { }
    ~Macro() { }

    const QString& getName() const { return name; }
    const QString& getValue() const { return value; }
    const QString& getFile() const { return file; }
    const uint getLine() const { return line; }

private:
    QString name;
    QString value;
    QString file;
    uint line;
} ;

class MacroTable
{
public:
    MacroTable() : defFileName(), defFileLine(0), macros()
    {
        macros.setAutoDelete(true);
    }
    ~MacroTable() { }

    bool addMacro(Macro* m);
    void setMacro(Macro* m);

    bool deleteMacro(const QString name)
    {
        return macros.remove(name);
    }

    void clear()
    {
        macros.clear();
    }
    QString resolve(const QStringList* argList);
    QString expandReportVariable(QString text, const QStringList* argList);
    Macro* getMacro(const QString& name) const { return macros[name]; }

    void setLocation(const QString& df, int dl)
    {
        defFileName = df;
        defFileLine = dl;
    }

private:
    bool evalExpression(const QString expr) const;
    void errorMessage(const char* txt, ... ) const;

    /* We store a file name and a line number in case we need this for
     * error reports or warnings. This is the location of the macro reference,
     * not the macro definitions. */
    QString defFileName;
    int defFileLine;

    QDict<Macro> macros;
} ;

#endif
