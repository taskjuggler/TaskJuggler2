/*
 * taskjuggler.h - TaskJuggler
 *
 * Copyright (c) 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _taskjuggler_h_
#define _taskjuggler_h_

#define TJURL "http://www.suse.de/~freitag/taskjuggler"

typedef enum TaskStatus 
{ 
	Undefined = 0, NotStarted, InProgressLate, InProgress, OnTime, 
	InProgressEarly, Finished
};

typedef enum AccountType { Cost, Revenue };

#endif

