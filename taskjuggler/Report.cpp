/*
 * Report.cpp - TaskJuggler
 *
 * Copyright (c) 2001 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <config.h>

#include "Project.h"
#include "Report.h"
#include "Utility.h"
#include "ExpressionTree.h"

Report::Report(Project* p, const QString& f, time_t s, time_t e) :
		project(p), fileName(f), start(s), end(e)
{
}

bool
Report::isHidden(Task* t)
{
	hide->clearSymbolTable();
	QStringList flags = t->getFlags();
	for (QStringList::Iterator it = flags.begin(); it != flags.end(); ++it)
		hide->registerSymbol(*it, 1);
	return hide->eval() != 0;
}



