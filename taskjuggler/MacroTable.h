/*
 * MacroTable.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _MacroTable_h_
#define _MacroTable_h_

#include <qstring.h>
#include <qstringlist.h>
#include <qdict.h>
#include <qvaluelist.h>

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
	Macro() { }	// don't use this

	QString name;
	QString value;
	QString file;
	uint line;
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
	Macro* getMacro(const QString& name) const { return macros[name]; }

private:
	QDict<Macro> macros;
	QPtrList<QStringList> argStack;
} ;

#endif
