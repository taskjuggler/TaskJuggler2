/*
 * TaskScenario.h - TaskJuggler
 *
 * Copyright (c) 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "TaskScenario.h"

TaskScenario::TaskScenario()
{
	start = 0;
	end = 0;
	startBuffer = -1.0;
	endBuffer = -1.0;
	startBufferEnd = 0;
	endBufferStart = 0;
	duration = 0.0;
	length = 0.0;
	effort = 0.0;
	startCredit = -1.0;
	endCredit = -1.0;
	complete = -1;
	scheduled = FALSE;
}

