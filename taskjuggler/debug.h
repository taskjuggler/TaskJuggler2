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

#ifndef _debug_h_
#define _debug_h_

#define DEBUGMODE DebugCtrl.getDebugMode()
#define DEBUGLEVEL DebugCtrl.getDebugLevel()

#define DEBUGPF(l) ((DEBUGMODE & 1) && DEBUGLEVEL >= l)	// Project File Reader
#define DEBUGPS(l) ((DEBUGMODE & 2) && DEBUGLEVEL >= l)	// Project Scheduler
#define DEBUGTS(l) ((DEBUGMODE & 4) && DEBUGLEVEL >= l)	// Task Scheduler
#define DEBUGRS(l) ((DEBUGMODE & 8) && DEBUGLEVEL >= l)	// Resource Scheduler
#define DEBUGEX(l) ((DEBUGMODE & 16) && DEBUGLEVEL >= l) // Expression Tree Ev.

class DebugController
{
public:
	DebugController() : debugLevel(0), debugMode(-1) { }
	~DebugController() { }
	
	void setDebugLevel(int l)
   	{
	   	debugLevel = l;
	}
	int getDebugLevel() const { return debugLevel; }
	
	void setDebugMode(int m)
	{
		debugMode = m;
	}
	int getDebugMode() const { return debugMode; }

protected:
	int debugLevel;
	int debugMode;
} ;

extern DebugController DebugCtrl;

#endif

