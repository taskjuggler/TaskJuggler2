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

#include <ctype.h>
#include <stdio.h>
#include "MacroTable.h"

bool
MacroTable::addMacro(Macro* macro)
{
	for (Macro* m = macros.first(); m != 0; m = macros.next())
		if (m->getName() == macro->getName())
			return FALSE;
	macros.append(macro);
	return TRUE;
}

QString
MacroTable::expand(const QString& name)
{
	fflush(stdout);
	if (isdigit(name[0].latin1()))
	{
		QStringList* sl = argStack.last();
		uint idx = name.toInt();
		if (sl == 0)
		{
			qWarning("Macro argument stack is empty.");
			return QString::null;
		}
		if ((idx <= 0) || (sl->count() <= idx - 1))
		{
			qWarning("Index %d for argument out of range [1 - %d]!\n",
					 idx, sl->count());
			return QString::null;
		}
		return (*sl)[idx - 1];
	}
	else
		for (Macro* m = macros.first(); m != 0; m = macros.next())
			if (m->getName() == name)
				return m->getValue();
	return QString::null;
}
