/*
 * Scenario.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef _Scenario_h_
#define _Scenario_h_

#include "CoreAttributes.h"
#include "ScenarioList.h"

class Project;
class QString;

class Scenario : public CoreAttributes
{
    friend int ScenarioList::compareItemsLevel(Scenario* r1, Scenario* r2,
                                               int level);
public:
    Scenario(Project* p, const QString& i, const QString& n, Scenario* p);
    virtual ~Scenario();

    virtual const char* getType() const { return "Scenario"; }

    Scenario* getParent() const { return (Scenario*) parent; }

    ScenarioListIterator getSubListIterator() const;
} ;

#endif

