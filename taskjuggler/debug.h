/*
 * debug.h - TaskJuggler
 *
 * Copyright (c) 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#define DEBUGPF(l) ((debugMode & 1) && debugLevel >= l)	// Project File Reader
#define DEBUGPS(l) ((debugMode & 2) && debugLevel >= l)	// Project Scheduler
#define DEBUGTS(l) ((debugMode & 4) && debugLevel >= l)	// Task Scheduler
#define DEBUGRS(l) ((debugMode & 8) && debugLevel >= l)	// Resource Scheduler

