/*
 * XMLReport.h - TaskJuggler
 *
 * Copyright (c) 2002 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef _XMLReport_h_
#define _XMLReport_h_

#include <Report.h>

class Project;
class QDomDocument;
class QDomElement;
class CustomAttributeDefinition;
class Scenario;
class Shift;
class Interval;
class TaskDependency;

class XMLReport : public Report
{
public:
    XMLReport(Project* p, const QString& f, const QString& df, int dl);
    virtual ~XMLReport();

    virtual const char* getType() const { return "XMLReport"; }

    bool generate();

    bool addTaskAttribute(const QString& ta);
    QStringList getTaskAttributes() const { return taskAttributes; }

    void setMasterFile(bool mf) { masterFile = mf; }

private:
    bool generateProjectProperty(QDomElement* n);
    bool generateCustomAttributeDeclaration(QDomElement* parentEl,
             const QString& propertyName,
             QDictIterator<CustomAttributeDefinition> it);
    bool generateScenario(QDomElement* parentEl, Scenario* scenario);

    bool generateGlobalVacationList(QDomElement* parentNode);
    bool generateShiftList(QDomElement* parentNode);
    bool generateShift(QDomElement* parentEl, const Shift*);
    bool generateWorkingHours(QDomElement* el,
                              const QPtrList<Interval>* const* wh);
    bool generateResourceList(QDomElement* parentNode, ResourceList& frl,
                              TaskList& ftl);
    bool generateResource(QDomElement* parentEl, ResourceList& frl,
                          TaskList& ftl, const Resource* r);
    bool generateTaskList(QDomElement* parentNode, TaskList& ftl,
                          ResourceList& frl);
    bool generateTask(QDomElement* parentEl, TaskList& ftl, const Task* task);
    bool generateDepList(QDomElement* el, TaskList& filteredTaskList,
                         const Task* task,
                         QPtrListIterator<TaskDependency> dl,
                         const char* tag);
    bool generateCustomAttributeValue(QDomElement* parentEl,
                                      const QString& id,
                                      const CoreAttributes* property);
    bool generateAllocate(QDomElement* el, const Task* t);

    bool generateBookingList(QDomElement* parentEl, TaskList& ftl,
                             ResourceList& frl);
    void genTextAttr(QDomElement* el, const QString& name, const QString& text);
    void genDoubleAttr(QDomElement* el, const QString& name, double val);
    void genLongAttr(QDomElement* el, const QString& name, long val);

    void genTextElement(QDomElement* parentEl, const QString& name,
                        const QString& text);
    void genDateElement(QDomElement* el, const QString& name, time_t val);
    void genTimeElement(QDomElement* el, const QString& name, time_t val);

    QDomDocument* doc;

    QStringList taskAttributes;

    // True if the file should be a standalone project (*.tjp file).
    bool masterFile;
};

#endif

