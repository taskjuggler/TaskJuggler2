/*
 * XMLFile.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <stdio.h>

#include <qdom.h>
#include <qtextstream.h>

#include "tjlib-internal.h"
#include "debug.h"
#include "XMLFile.h"
#include "ParserNode.h"
#include "ParserElement.h"
#include "ParserTreeContext.h"
#include "Project.h"
#include "Utility.h"
#include "Scenario.h"
#include "Shift.h"
#include "Task.h"
#include "TaskScenario.h"
#include "Allocation.h"
#include "VacationInterval.h"

ParserNode* XMLFile::parserRootNode = 0;

XMLFile::XMLFile(Project* p) :
    project(p)
{
    if (!parserRootNode)
        createParseTree();

    doc = 0;
}

XMLFile::~XMLFile()
{
    delete doc;
}

void
XMLFile::createParseTree()
{
    /* Build the tree that describes how we expect the DOM tree to look like.
     * Each node of the tree can have an arbitrary number of elements. Each
     * element may then again have a node, when it has sub-elements. */
    parserRootNode = new ParserNode();
    ParserElement* pe = 
        new ParserElement("taskjuggler", &XMLFile::doTaskJuggler,
                          parserRootNode); 
    ParserNode* taskjugglerNode = new ParserNode(pe);
    {
        pe = new ParserElement("project", &XMLFile::doProject, taskjugglerNode);
        ParserNode* projectNode = new ParserNode(pe);
        {
            new ParserElement("start", &XMLFile::doProjectStart, projectNode);
            new ParserElement("end", &XMLFile::doProjectEnd, projectNode);
            new ParserElement("now", &XMLFile::doProjectNow, projectNode);
            new ParserElement("extend", 0, projectNode);
            new ParserElement("currencyFormat", &XMLFile::doCurrencyFormat,
                              projectNode);
            new ParserElement("workingHours", 0, projectNode);
            pe = new ParserElement("scenario", &XMLFile::doScenario,
                                   projectNode);
            ParserNode* scenarioNode = new ParserNode(pe);
            {
                scenarioNode->add(pe, "scenario");
            }
        }

        pe = new ParserElement("vacationList", 0, taskjugglerNode);
        ParserNode* vacationListNode = new ParserNode(pe);
        {
            pe = new ParserElement("vacation", &XMLFile::doVacation,
                                   vacationListNode);
            ParserNode* vacationNode = new ParserNode(pe);
            {
                new ParserElement("start", &XMLFile::doVacationStart,
                                  vacationNode);
                new ParserElement("end", &XMLFile::doVacationEnd,
                                  vacationNode);
            }
        }

        pe = new ParserElement("shiftList", 0, taskjugglerNode);
        ParserNode* shiftListNode = new ParserNode(pe);
        {
            pe = new ParserElement("shift", &XMLFile::doShift,
                                   shiftListNode);
            ParserNode* shiftNode = new ParserNode(pe);
            {
                shiftNode->add(pe, "shift");    // recursive link
                new ParserElement("workingHours", 0, shiftListNode);
            }
        }
        
        pe = new ParserElement("taskList", 0, taskjugglerNode);
        ParserNode* taskListNode = new ParserNode(pe);
        {
            pe = new ParserElement("task", &XMLFile::doTask, taskListNode);
            ParserNode* taskNode = new ParserNode(pe);

            taskNode->add(pe, "task");   // recursive link
            pe = new ParserElement("taskScenario", &XMLFile::doTaskScenario,
                                   taskNode);
            ParserNode* taskScenarioNode = new ParserNode(pe);
            {
                new ParserElement("customScenario", 0, taskScenarioNode);
                new ParserElement("start", &XMLFile::doTaskScenarioStart,
                                  taskScenarioNode);
                new ParserElement("end", &XMLFile::doTaskScenarioEnd,
                                  taskScenarioNode);
                new ParserElement("maxEnd", &XMLFile::doTaskScenarioMaxEnd,
                                  taskScenarioNode);
                new ParserElement("maxStart", &XMLFile::doTaskScenarioMaxStart,
                                  taskScenarioNode);
                new ParserElement("minEnd", &XMLFile::doTaskScenarioMinEnd,
                                  taskScenarioNode);
                new ParserElement("minStart", &XMLFile::doTaskScenarioMinStart,
                                  taskScenarioNode);
                
            }
            pe = new ParserElement("allocate", &XMLFile::doAllocate,
                                   taskNode);
            ParserNode* allocateNode = new ParserNode(pe);
            {
                new ParserElement("candidate", &XMLFile::doCandidate,
                                  allocateNode);
            }
            pe = new ParserElement("flag", &XMLFile::doFlag, taskNode);
        }

        pe = new ParserElement("resourceList", 0, taskjugglerNode);
        ParserNode* resourceListNode = new ParserNode(pe);
        {
            pe = new ParserElement("resource", 0, resourceListNode);
            ParserNode* resourceNode = new ParserNode(pe);

            resourceNode->add(pe, "resource");  // recursive link
        }
    }
}

bool
XMLFile::readDOM(const QString& file, const QString&, const QString&,
                 bool masterfile)
{
    if (masterfile)
        masterFile = file;
    
    FILE* fh;
    QTextStream* f;    
    if (file == ".")
    {
        f = new QTextStream(stdin, IO_ReadOnly);
        fh = stdin;
    }
    else
    {
        if ((fh = fopen(file, "r")) == 0)
        {
            qWarning(i18n("Cannot open file '%1'.").arg(file));
            return FALSE;
        }
        f = new QTextStream(fh, IO_ReadOnly);
    }

    if (DEBUGLEVEL > 0)
        qWarning(i18n("Processing file \'%1\'").arg(file));

    doc = new QDomDocument(file);
    QString buf(f->read());
    
    if (!doc->setContent(buf))
    {
        qWarning(i18n("Syntax error in XML file '%1'.").arg(file));
        return FALSE;
    }
    fclose(fh);
    delete f;
    
    return TRUE;
}

bool
XMLFile::parse()
{
    QDomNode n = doc->firstChild();

    ParserTreeContext ptc;
    ptc.setTask(0);

    return parseNode(parserRootNode, n, ptc);
}

bool
XMLFile::parseNode(const ParserNode* pn, QDomNode n, ParserTreeContext ptc)
{
    while (!n.isNull())
    {
        qDebug("Traversing node %s", n.nodeName().latin1());
        QDomElement el = n.toElement();
        if (!el.isNull())
        {
            const ParserElement* pEl = pn->getElement(el.tagName());
            if (!pEl)
            {
                qWarning(i18n("Unsupported XML element '%1' in node '%2' "
                              "of '%2'. Parser is at node %3")
                         .arg(el.tagName()).arg(n.nodeName()).arg(masterFile)
                         .arg(pn->getParentElement()->getTag()));
            }
            else
            {
                /* Create a copy of the current ptc. The node function may
                 * modify this copy to pass contextual information to the
                 * elements of this node. */
                ParserTreeContext ptcCopy = ptc;
                /* If a node function has been specified, call this function
                 * and pass it the ptc. */
                if (pEl->getFunc())
                    if (!((*this).*(pEl->getFunc()))(n, ptcCopy))
                        return FALSE;
                /* If sub-elements of this node have been defined in the
                 * parse tree, call this function again to process the
                 * sub-elements. */
                if (pEl->getNode())
                    if (!parseNode(pEl->getNode(), n.firstChild(), ptcCopy))
                        return FALSE;
            }
        }
        qDebug("Node %s done.", n.nodeName().latin1());
        n = n.nextSibling();
    }

    return TRUE;
}

bool
XMLFile::doTaskJuggler(QDomNode&, ParserTreeContext&)
{
    return TRUE;
}

bool
XMLFile::doProject(QDomNode& n, ParserTreeContext&)
{
    QDomElement el = n.toElement();
    // mandatory attributes
    project->addId(el.attribute("id"));
    project->setName(el.attribute("name"));
    project->setVersion(el.attribute("version"));
    project->setTimeZone(el.attribute("timezone"));
   
    // optional attributes
    project->setScheduleGranularity
        (el.attribute("timingResolution", "3600").toInt());
    project->setDailyWorkingHours
        (el.attribute("dailyWorkingHours", "8").toDouble());
    project->setYearlyWorkingDays
        (el.attribute("yearlyWorkingDays", "260.714").toDouble());
    project->setWeekStartsMonday
        (el.attribute("weekStartMonday", "1").toInt());
    project->setTimeFormat(el.attribute("timeFormat", "%Y-%m-%d %H:%M"));
    project->setShortTimeFormat(el.attribute("shortTimeFormat", "%H:%M"));
    
    return TRUE;
}

bool
XMLFile::doProjectStart(QDomNode& n, ParserTreeContext&)
{
    project->setStart(n.toElement().text().toLong());
    return TRUE;
}

bool
XMLFile::doProjectEnd(QDomNode& n, ParserTreeContext&)
{
    project->setEnd(n.toElement().text().toLong());
    return TRUE;
}

bool
XMLFile::doProjectNow(QDomNode& n, ParserTreeContext&)
{
    project->setNow(n.toElement().text().toLong());
    return TRUE;
}

bool
XMLFile::doScenario(QDomNode& n, ParserTreeContext& ptc)
{
    QDomElement el = n.toElement();

    /* The project has a default scenario called plan. The XML always brings
     * it's own scenario definition. So we have to clear the default. */
    if (!ptc.getScenario())
        delete project->getScenario(0);
    Scenario* scenario = new Scenario(project, el.attribute("id"), 
                                      el.attribute("name"), ptc.getScenario());
    ptc.setScenario(scenario);

    return TRUE;
}

bool
XMLFile::doCurrencyFormat(QDomNode&, ParserTreeContext&)
{
    return TRUE;
}

bool
XMLFile::doVacation(QDomNode& n, ParserTreeContext& ptc)
{
    QDomElement el = n.toElement();
    VacationInterval* vi = new VacationInterval();
    vi->setName(el.attribute("name"));
    ptc.setVacationInterval(vi);
    return TRUE;
}

bool
XMLFile::doVacationStart(QDomNode& n, ParserTreeContext& ptc)
{
    QDomElement el = n.toElement();
    
    ptc.getVacationInterval()->setStart(el.text().toLong());

    return TRUE;
}

bool
XMLFile::doVacationEnd(QDomNode& n, ParserTreeContext& ptc)
{
    QDomElement el = n.toElement();

    ptc.getVacationInterval()->setEnd(el.text().toLong());

    return TRUE;
}

bool
XMLFile::doShift(QDomNode& n, ParserTreeContext& ptc)
{
    QDomElement el = n.toElement();
    Shift* shift = new Shift(project, el.attribute("id"),
                             el.attribute("name"), ptc.getShift());
    ptc.setShift(shift);

    return TRUE;
}

bool
XMLFile::doTask(QDomNode& n, ParserTreeContext& ptc)
{
    QDomElement el = n.toElement();
    Task* t = new Task(project, el.attribute("id"), el.attribute("name"),
                       ptc.getTask(), "", 0);
    ptc.setTask(t);
    t->setProjectId(el.attribute("projectId"));
    t->setMilestone(el.attribute("milestone").toInt());
    t->setScheduling(el.attribute("asapScheduling").toInt() ?
                     Task::ASAP : Task::ALAP);
    t->setPriority(el.attribute("priority").toInt());
    t->setResponsible(project->getResource(el.attribute("responsible")));
    t->setAccount(project->getAccount(el.attribute("account")));
    
    return TRUE;
}

bool
XMLFile::doTaskScenario(QDomNode& n, ParserTreeContext& ptc)
{
    QDomElement el = n.toElement();
    int sc = project->getScenarioIndex(el.attribute("scenarioId")) - 1;
    ptc.setScenarioIndex(sc);
    Task* t = ptc.getTask();
    t->setEffort(sc, el.attribute("effort", "0.0").toDouble());
    t->setDuration(sc, el.attribute("duration", "0.0").toDouble());
    t->setLength(sc, el.attribute("length", "0.0").toDouble());
    t->setScheduled(sc, el.attribute("scheduled", "0").toInt());
    t->setComplete(sc, el.attribute("complete", "-1").toInt()); 
    
    return TRUE;
}

bool
XMLFile::doTaskScenarioStart(QDomNode& n, ParserTreeContext& ptc)
{
    qDebug("Setting start of %s to %s", ptc.getTask()->getId().latin1(),
           time2ISO(n.toElement().text().toLong()).latin1());
    ptc.getTask()->setStart(ptc.getScenarioIndex(),
                            n.toElement().text().toLong());
    return TRUE;
}

bool
XMLFile::doTaskScenarioEnd(QDomNode& n, ParserTreeContext& ptc)
{
    ptc.getTask()->setEnd(ptc.getScenarioIndex(),
                          n.toElement().text().toLong());
    return TRUE;
}

bool
XMLFile::doTaskScenarioMaxEnd(QDomNode& n, ParserTreeContext& ptc)
{
    ptc.getTask()->setMaxEnd(ptc.getScenarioIndex(),
                             n.toElement().text().toLong());
    return TRUE;
}

bool
XMLFile::doTaskScenarioMinEnd(QDomNode& n, ParserTreeContext& ptc)
{
    ptc.getTask()->setMinEnd(ptc.getScenarioIndex(),
                             n.toElement().text().toLong());
    return TRUE;
}

bool
XMLFile::doTaskScenarioMaxStart(QDomNode& n, ParserTreeContext& ptc)
{
    ptc.getTask()->setMaxStart(ptc.getScenarioIndex(),
                               n.toElement().text().toLong());
    return TRUE;
}

bool
XMLFile::doTaskScenarioMinStart(QDomNode& n, ParserTreeContext& ptc)
{
    ptc.getTask()->setMinStart(ptc.getScenarioIndex(),
                               n.toElement().text().toLong());
    return TRUE;
}

bool
XMLFile::doAllocate(QDomNode&, ParserTreeContext& ptc)
{
    Allocation* a = new Allocation();
    ptc.getTask()->addAllocation(a);
    ptc.setAllocation(a);

    return TRUE;
}

bool
XMLFile::doCandidate(QDomNode& n, ParserTreeContext& ptc)
{
    QDomElement el = n.toElement();
    ptc.getAllocation()->addCandidate
        (project->getResource(el.attribute("resourceId")));

    return TRUE;
}

bool
XMLFile::doFlag(QDomNode& n, ParserTreeContext& ptc)
{
    QDomElement el = n.toElement();

    ptc.getCoreAttributes()->addFlag(el.text());

    return TRUE;
}

