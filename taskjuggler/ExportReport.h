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
        QDictIterator<CustomAttributeDefinition> it);
    bool generateShiftList();
    bool generateWorkingHours(const QPtrList<const Interval>* const* wh);
    bool generateResourceList(ResourceList& frl, TaskList& ftl);
    bool generateResource(ResourceList& frl, Resource* r, int ident);
    bool generateTaskList(TaskList& ftl, ResourceList& frl);
    bool generateTask(TaskList& ftl, Task* task, int indent);
    bool generateTaskAttributeList(TaskList& ftl);
    bool generateTaskSupplement(TaskList& ftl, Task* task, int indent);
    bool generateResourceAttributesList(TaskList& ftl, ResourceList& frl);
    
    QStringList taskAttributes;

    // True if the file should be a standalone project (*.tjp file). 
    bool masterFile;
};

#endif

