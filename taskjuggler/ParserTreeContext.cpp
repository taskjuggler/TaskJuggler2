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
#include "Resource.h"
#include "Task.h"
#include "VacationInterval.h"

Scenario*
ParserTreeContext::getScenario() const
{
    return dynamic_cast<Scenario*>(ca);
}

Shift*
ParserTreeContext::getShift() const
{
    return dynamic_cast<Shift*>(ca);
}

Resource*
ParserTreeContext::getResource() const
{
    return dynamic_cast<Resource*>(ca);
}

Task*
ParserTreeContext::getTask() const
{
    return dynamic_cast<Task*>(ca);
}

VacationInterval*
ParserTreeContext::getVacationInterval() const
{
    return dynamic_cast<VacationInterval*>(interval);
}

