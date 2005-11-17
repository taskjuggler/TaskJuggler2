/*
 * ScenarioList.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _ScenarioList_h_
#define _ScenarioList_h_

#include "CoreAttributesList.h"

class QString;
class Scenario;

/**
 * @short A list of scenarios.
 * @author Chris Schlaeger <cs@kde.org>
 */
class ScenarioList : public CoreAttributesList
{
public:
    ScenarioList();
    virtual ~ScenarioList() { }

    Scenario* getScenario(const QString& id) const;

    static bool isSupportedSortingCriteria(int sc);
    
    virtual int compareItemsLevel(Scenario* r1, Scenario* r2, int level);

    virtual Scenario* operator[](int i)
    {
        return (Scenario*) at(i);
    }

protected:
    virtual int compareItems(QCollection::Item i1, QCollection::Item i2);
} ;

/**
 * @short Iterator class for ScenarioList objects.
 * @author Chris Schlaeger <cs@kde.org>
 */
class ScenarioListIterator : public virtual CoreAttributesListIterator 
{
public:
    ScenarioListIterator(const CoreAttributesList& l) :
        CoreAttributesListIterator(l) { }
    ~ScenarioListIterator() { }
    Scenario* operator*() { return (Scenario*) get(); }
} ;

#endif
