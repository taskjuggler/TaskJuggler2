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
#include "Task.h"
#include "TaskScenario.h"

ParserNode* XMLFile::parserRootNode = 0;

XMLFile::XMLFile(Project* p) :
    project(p)
{
    if (!parserRootNode)
        createNodeTree();

    doc = 0;
}

XMLFile::~XMLFile()
{
    delete doc;
}

void
XMLFile::createNodeTree()
{
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
            new ParserElement("scenario", 0, projectNode);
        }

        pe = new ParserElement("vacationList", 0, taskjugglerNode);
        ParserNode* vacationListNode = new ParserNode(pe);
        {
            pe = new ParserElement("vacation", 0, vacationListNode);
            ParserNode* vacationNode = new ParserNode(pe);

            vacationNode->add(pe, "vacation");  // recursive link
        }

        pe = new ParserElement("taskList", 0, taskjugglerNode);
        ParserNode* taskListNode = new ParserNode(pe);
        {
            pe = new ParserElement("task", &XMLFile::doTask, taskListNode);
            ParserNode* taskNode = new ParserNode(pe);

            taskNode->add(pe, "task");   // recursive link
            pe = new ParserElement("taskScenario", &XMLFile::doTaskScenario,
                                   taskListNode);
            ParserNode* taskScenarioNode = new ParserNode(pe);

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
    ptc.setParentTask(0);

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
                              "of '%2'")
                         .arg(el.tagName()).arg(n.nodeName()).arg(masterFile));
    //            return FALSE;
            }
            else
            {
                ParserTreeContext ptcCopy = ptc;
                if (pEl->getFunc())
                    if (!((*this).*(pEl->getFunc()))(n, ptcCopy))
                        return FALSE;
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
XMLFile::doCurrencyFormat(QDomNode&, ParserTreeContext&)
{
    return TRUE;
}

bool
XMLFile::doTask(QDomNode& n, ParserTreeContext& ptc)
{
    QDomElement el = n.toElement();
    Task* t = new Task(project, el.attribute("id"), el.attribute("name"),
                       ptc.getParentTask(), "", 0);
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
    return TRUE;
}

bool
XMLFile::doTaskScenarioStart(QDomNode& n, ParserTreeContext& ptc)
{
    ptc.getTaskScenario()->setStart(n.toElement().text().toLong());
    return TRUE;
}

bool
XMLFile::doTaskScenarioEnd(QDomNode& n, ParserTreeContext& ptc)
{
    ptc.getTaskScenario()->setEnd(n.toElement().text().toLong());
    return TRUE;
}

bool
XMLFile::doTaskScenarioMaxEnd(QDomNode& n, ParserTreeContext& ptc)
{
    ptc.getTaskScenario()->setMaxEnd(n.toElement().text().toLong());
    return TRUE;
}

bool
XMLFile::doTaskScenarioMinEnd(QDomNode& n, ParserTreeContext& ptc)
{
    ptc.getTaskScenario()->setMinEnd(n.toElement().text().toLong());
    return TRUE;
}

bool
XMLFile::doTaskScenarioMaxStart(QDomNode& n, ParserTreeContext& ptc)
{
    ptc.getTaskScenario()->setMaxStart(n.toElement().text().toLong());
    return TRUE;
}

bool
XMLFile::doTaskScenarioMinStart(QDomNode& n, ParserTreeContext& ptc)
{
    ptc.getTaskScenario()->setMinStart(n.toElement().text().toLong());
    return TRUE;
}

