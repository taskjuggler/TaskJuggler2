/*
 * MacroTable.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
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
#include <qptrlist.h>

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
	void clear()
	{
		macros.clear();
		argStack.clear();
	}
	QString resolve(const QString& name) const;
	QString expand(const QString& text) const;
	Macro* getMacro(const QString& name) const { return macros[name]; }

	void setLocation(const QString& df, int dl)
	{
		defFileName = df;
		defFileLine = dl;
	}

private:
	void errorMessage(const char* txt, ... ) const;

	/* We store a file name and a line number in case we need this for
	 * error reports or warnings. */
	QString defFileName;
	int defFileLine;
	
	QDict<Macro> macros;
	QPtrList<QStringList> argStack;
} ;

#endif
