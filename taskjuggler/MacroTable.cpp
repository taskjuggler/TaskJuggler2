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

#include <ctype.h>
#include <stdio.h>
#include "MacroTable.h"

bool
MacroTable::addMacro(Macro* macro)
{
	if (macros[macro->getName()])
		return FALSE;
	macros.insert(macro->getName(), macro);
	return TRUE;
}

QString
MacroTable::resolve(const QString& name)
{
	fflush(stdout);
	if (isdigit(name[0].latin1()))
	{
		QStringList* sl = argStack.at(argStack.count() - 2);
		uint idx = name.toInt();
		if (sl == 0)
		{
			warningMsg("Macro argument stack is empty.");
			return QString::null;
		}
		if ((idx <= 0) || (sl->count() <= idx - 1))
		{
			warningMsg("Index %d for argument out of range [1 - %d]!\n",
					   idx, sl->count());
			return QString::null;
		}
		return (*sl)[idx - 1];
	}
	else
		if (macros[name])
			return macros[name]->getValue();

	warningMsg("Usage of undefined macro '%s'", name.latin1());

	return QString::null;
}

QString
MacroTable::expand(const QString& text)
{
	QString res;
	for (uint i = 0; i < text.length(); i++)
	{
		if (text[i] == '$')
		{
			if (i + 1 >= text.length() || text[i + 1] != '{')
			{
				res += '$';
				continue;
			}
			uint cb;
			for (cb = 1; cb < text.length() && text[cb] != '}'; cb++)
				;
			res += expand(resolve(text.mid(i + 2, cb - (i + 2))));
			i = cb;
		}
		else
			res += text[i];
	}
	return res;
}

void
MacroTable::warningMsg(const char* msg, ... )
{
	va_list ap;
	va_start(ap, msg);
	char buf[1024];
	vsnprintf(buf, 1024, msg, ap);
	va_end(ap);
	qWarning("%s:%d:%s", defFileName.latin1(), defFileLine, buf);
}


