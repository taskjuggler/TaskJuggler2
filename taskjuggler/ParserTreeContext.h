/*
 * ParserTreeContext.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _ParserTreeContext_h_
#define _ParserTreeContext_h_

class CoreAttributes;
class Scenario;
class Shift;
class Task;
class Resource;
class TaskScenario;
class Allocation;
class VacationInterval;

class ParserTreeContext
{
public:
    ParserTreeContext() { }
    ~ParserTreeContext() { }

    void setCoreAttributes(CoreAttributes* c) { ca = c; }
    CoreAttributes* getCoreAttributes() const { return ca; }

    void setScenario(Scenario* s) { ca = (CoreAttributes*) s; }
    Scenario* getScenario();

    void setShift(Shift* s) { ca = (CoreAttributes*) s; }
    Shift* getShift();

    void setTask(Task* t) { ca = (CoreAttributes*) t; }
    Task* getTask();

    void setScenarioIndex(int i) { scenarioIndex = i; }
    int getScenarioIndex() const { return scenarioIndex; }

    void setAllocation(Allocation* a) { allocation = a; }
    Allocation* getAllocation() const { return allocation; }

    void setVacationInterval(VacationInterval* v) { vacationInterval = v; }
    VacationInterval* getVacationInterval() const { return vacationInterval; }

private:
    CoreAttributes* ca;
    int scenarioIndex;
    Allocation* allocation;
    VacationInterval* vacationInterval;
} ;

#endif

