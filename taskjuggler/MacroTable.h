/*
 * MacroTable.h - TaskJuggler
 *
 * Copyright (c) 2001 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _MacroTable_h_
#define _MacroTable_h_

#include <qstring.h>
#include <qstringlist.h>
#include <qlist.h>
#include <qvaluelist.h>

class Macro
{
public:
	Macro(const QString& n, const QString& v)
		: name(n), value(v) { }
	~Macro() { }

	const QString& getName() { return name; }
	const QString& getValue() { return value; }

private:
	Macro() { }	// don't use this

	QString name;
	QString value;
} ;

class MacroTable
{
public:
	MacroTable()
	{
		macros.setAutoDelete(TRUE);
		argStack.setAutoDelete(TRUE);
	}
	~MacroTable() { }

	bool addMacro(Macro* m);
	void pushArguments(QStringList* sl)
	{
		argStack.append(sl);
	}
	void popArguments()
	{
		argStack.removeLast();
	}
	QString expand(const QString& name);

private:
	QList<Macro> macros;
	QList<QStringList> argStack;
} ;

#endif
