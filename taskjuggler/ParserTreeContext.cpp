/*
 * ParserTreeContext.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "ParserTreeContext.h"
#include "Scenario.h"
#include "Shift.h"
#include "Task.h"

Scenario*
ParserTreeContext::getScenario()
{
    return dynamic_cast<Scenario*>(ca);
}

Shift*
ParserTreeContext::getShift()
{
    return dynamic_cast<Shift*>(ca);
}
        
Task*
ParserTreeContext::getTask()
{
    return dynamic_cast<Task*>(ca);
}

