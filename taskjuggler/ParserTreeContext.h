/*
 * ParserTreeContext.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006
 * by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _ParserTreeContext_h_
#define _ParserTreeContext_h_

#include <qptrlist.h>
#include <qstring.h>

class CoreAttributes;
class Scenario;
class Shift;
class Resource;
class Account;
class Task;
class Resource;
class TaskScenario;
class Allocation;
class VacationInterval;
class Interval;
class TaskDependency;

class ParserTreeContext
{
public:
    ParserTreeContext() { }
    ~ParserTreeContext() { }

    void setCoreAttributes(CoreAttributes* c) { ca = c; }
    CoreAttributes* getCoreAttributes() const { return ca; }

    void setScenario(Scenario* s) { ca = (CoreAttributes*) s; }
    Scenario* getScenario() const;

    void setWeekday(int day) { weekday = day; }
    int getWeekday() const { return weekday; }

    void setShift(Shift* s) { ca = (CoreAttributes*) s; }
    Shift* getShift() const;

    void setResource(Resource* r) { ca = (CoreAttributes*) r; }
    Resource* getResource() const;

    void setAccount(Account* a) { ca = (CoreAttributes*) a; }
    Account* getAccount() const;

    void setTask(Task* t) { ca = (CoreAttributes*) t; }
    Task* getTask() const;

    void setScenarioIndex(int i) { scenarioIndex = i; }
    int getScenarioIndex() const { return scenarioIndex; }

    void setAllocation(Allocation* a) { allocation = a; }
    Allocation* getAllocation() const { return allocation; }

    void setWorkingHours(QPtrList<Interval>* whs) { workingHours = whs; }
    QPtrList<Interval>* getWorkingHours() const { return workingHours; }

    void setTaskDependency(TaskDependency* td) { taskDependency = td; }
    TaskDependency* getTaskDependency() const { return taskDependency; }

    void setInterval(Interval* iv) { interval = iv; }
    Interval* getInterval() const { return interval; }

    void setVacationInterval(VacationInterval* v)
    {
        interval = (Interval*) v;
    }
    VacationInterval* getVacationInterval() const;

    void setExtendProperty(const QString& ep) { extendProperty = ep; }
    const QString& getExtendProperty() const { return extendProperty; }

private:
    CoreAttributes* ca;
    int scenarioIndex;
    int weekday;
    Allocation* allocation;
    TaskDependency* taskDependency;
    Interval* interval;
    QPtrList<Interval>* workingHours;
    QString extendProperty;
} ;

#endif

