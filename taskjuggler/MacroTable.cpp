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

#include <ctype.h>
#include <stdio.h>

#include "TjMessageHandler.h"
#include "tjlib-internal.h"
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
MacroTable::resolve(const QString& name) const
{
	if (isdigit(name[0].latin1()))
	{
		QPtrListIterator<QStringList> pli(argStack);
		pli.toLast();
		--pli;
		QStringList* sl = *pli;
		uint idx = name.toInt();
		if (sl == 0)
		{
			errorMessage(i18n("Macro argument stack is empty."));
			return QString::null;
		}
		if ((idx <= 0) || (sl->count() <= idx - 1))
		{
			errorMessage
				(i18n("Index %1 for argument out of range [1 - %2]!")
				 .arg(idx).arg(sl->count()));
			return QString::null;
		}
		return (*sl)[idx - 1];
	}
	else
		if (macros[name])
			return macros[name]->getValue();

	errorMessage
		(i18n("Usage of undefined macro '%1'").arg(name));

	return QString::null;
}

QString
MacroTable::expand(const QString& text) const
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
			for (cb = i + 2; cb < text.length() && text[cb] != '}'; cb++)
				;
			if (text[cb] != '}')
			{
				errorMessage
					(i18n("Unterminated macro call '%1'").arg(text));
				return res;
			}
			res += expand(resolve(text.mid(i + 2, cb - (i + 2))));
			i = cb;
		}
		else
			res += text[i];
	}
	return res;
}

void
MacroTable::errorMessage(const char* msg, ... ) const
{
	va_list ap;
	va_start(ap, msg);
	char buf[1024];
	vsnprintf(buf, 1024, msg, ap);
	va_end(ap);

	TJMH.errorMessage(buf, defFileName, defFileLine);
}


