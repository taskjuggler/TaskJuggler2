/*
 * ExportReport.h - TaskJuggler
 *
 * Copyright (c) 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef _ExportReport_h_
#define _ExportReport_h_

#include <qstring.h>
#include <qdict.h>

#include <Report.h>

class Project;
class CustomAttributeDefinition;

class ExportReport : public Report
{
public:
    ExportReport(Project* p, const QString& f, const QString& df, int dl);
    virtual ~ExportReport() { }

    bool generate();

    bool addTaskAttribute(const QString& ta);
    QStringList getTaskAttributes() const { return taskAttributes; }

    void setMasterFile(bool mf) { masterFile = mf; }
   
private:
    ExportReport() { }

    bool generateProjectProperty();
    bool generateCustomAttributeDeclaration(const QString& propertyName,
             QDictIterator<const CustomAttributeDefinition> it);
    bool generateScenario(const Scenario* scenario, int indent);
    bool generateShiftList();
    bool generateShift(const Shift*, int indent);
    bool generateWorkingHours(const QPtrList<const Interval>* const* wh,
                              const QPtrList<const Interval>* const* ref,
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
};

#endif

