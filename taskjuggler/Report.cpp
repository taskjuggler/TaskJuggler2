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
	hideTask = 0;
	hideResource = 0;
}

Report::~Report()
{
	delete hideTask;
	delete hideResource;
}

bool
Report::isTaskHidden(Task* t)
{
	if (!hideTask)
		return FALSE;

	hideTask->clearSymbolTable();
	QStringList flags = *t;
	for (QStringList::Iterator it = flags.begin(); it != flags.end(); ++it)
		hideTask->registerSymbol(*it, 1);
	return hideTask->eval() != 0;
}

bool
Report::isResourceHidden(Resource* r)
{
	if (!hideResource)
		return FALSE;

	hideResource->clearSymbolTable();
	QStringList flags = *r;
	for (QStringList::Iterator it = flags.begin(); it != flags.end(); ++it)
		hideResource->registerSymbol(*it, 1);
	return hideResource->eval() != 0;
}


