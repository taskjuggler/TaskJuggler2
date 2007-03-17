/*
 * ExportReport.h - TaskJuggler
 *
 * Copyright (c) 2002 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef _ExportReport_h_
#define _ExportReport_h_

#include <Report.h>
#include <TaskDependency.h>

class Project;
class CustomAttributeDefinition;
class Scenario;
class Shift;

class ExportReport : public Report
{
public:
    ExportReport(Project* p, const QString& f, const QString& df, int dl);
    virtual ~ExportReport() { }

    virtual const char* getType() const { return "ExportReport"; }

    bool generate();

    bool addTaskAttribute(const QString& ta);
    QStringList getTaskAttributes() const { return taskAttributes; }

    void setMasterFile(bool mf) { masterFile = mf; }

    void resetContentFlags();
    void setListShifts(bool ls) { listShifts = ls; }
    void setListResources(bool lr) { listResources = lr; }
    void setListTasks(bool lt) { listTasks = lt; }
    void setListBookings(bool lb) { listBookings = lb; }

private:
    bool generateProjectProperty();
    bool generateCustomAttributeDeclaration(const QString& propertyName,
             QDictIterator<CustomAttributeDefinition> it);
    bool generateScenario(const Scenario* scenario, int indent);
    bool generateShiftList();
    bool generateShift(const Shift*, int indent);
    bool generateWorkingHours(const QPtrList<Interval>* const* wh,
                              const QPtrList<Interval>* const* ref,
                              int indent);
    bool generateProjectIds(const TaskList& tasks);
    bool generateResourceList(ResourceList& frl, TaskList& ftl);
    bool generateResource(ResourceList& frl, const Resource* r, int ident);
    bool generateTaskList(TaskList& ftl, ResourceList& frl);
    bool generateTask(TaskList& ftl, const Task* task, int indent);
    bool generateTaskAttributeList(TaskList& ftl);
    bool generateTaskSupplement(TaskList& ftl, const Task* task, int indent);
    bool generateDepList(TaskList& filteredTaskList, const Task* task,
                         QPtrListIterator<TaskDependency> depIt,
                         const char* tag, int indent);
    bool generateResourceAttributesList(TaskList& ftl, ResourceList& frl);
    bool generateCustomAttributeValue(const QString& id,
                                      const CoreAttributes* property,
                                      int indent);

    QStringList taskAttributes;

    // True if the file should be a standalone project (*.tjp file).
    bool masterFile;

    bool listShifts;
    bool listTasks;
    bool listResources;
    bool listBookings;
};

#endif

