/*
 * XMLFile.cpp - TaskJuggler
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

#include <stdio.h>
#include <unistd.h>
#include <zlib.h>

#include <qdom.h>
#include <qtextstream.h>

#include "tjlib-internal.h"
#include "debug.h"
#include "ParserNode.h"
#include "ParserElement.h"
#include "ParserTreeContext.h"
#include "Project.h"
#include "Utility.h"
#include "Scenario.h"
#include "Shift.h"
#include "Resource.h"
#include "Account.h"
#include "Task.h"
#include "TaskScenario.h"
#include "Allocation.h"
#include "VacationInterval.h"
#include "Booking.h"
#include "CustomAttributeDefinition.h"
#include "ReferenceAttribute.h"
#include "TextAttribute.h"

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
     * element may then again have a node, when it has sub-elements. The
     * structure of the tree built here very closely resembles the
     * taskjuggler.dtd. */
    parserRootNode = new ParserNode();
    ParserElement* pe =
        new ParserElement("taskjuggler", &XMLFile::doTaskJuggler,
                          parserRootNode);
    ParserNode* taskjugglerNode = new ParserNode(pe);
    {
        // Project Element
        pe = new ParserElement("project", &XMLFile::doProject, taskjugglerNode);
        ParserNode* projectNode = new ParserNode(pe);
        {
            new ParserElement("start", &XMLFile::doProjectStart, projectNode);
            new ParserElement("end", &XMLFile::doProjectEnd, projectNode);
            new ParserElement("now", &XMLFile::doProjectNow, projectNode);
            pe = new ParserElement("extend", &XMLFile::doExtend, projectNode);
            ParserNode* extendNode = new ParserNode(pe);
            {
                new ParserElement("extendAttributeDefinition",
                                  &XMLFile::doExtendAttribute,
                                  extendNode);
            }
            new ParserElement("currencyFormat", &XMLFile::doCurrencyFormat,
                              projectNode);
            pe = new ParserElement("workingHours", 0, projectNode);
            createSubTreeWorkingHours
                (&XMLFile::doProjectWeekdayWorkingHours, pe,
                 &XMLFile::doProjectWeekdayWorkingHoursPost);
            pe = new ParserElement("scenario", &XMLFile::doScenario,
                                   projectNode);
            ParserNode* scenarioNode = new ParserNode(pe);
            {
                scenarioNode->add(pe, "scenario");
            }
        }

        // vacationlist element
        createSubTreeVacationList(&XMLFile::doProjectVacation,
                                  taskjugglerNode);

        // shiftList element
        pe = new ParserElement("shiftList", &XMLFile::doShiftList,
                               taskjugglerNode);
        ParserNode* shiftListNode = new ParserNode(pe);
        {
            pe = new ParserElement("shift", &XMLFile::doShift,
                                   shiftListNode);
            ParserNode* shiftNode = new ParserNode(pe);
            {
                shiftNode->add(pe, "shift");    // recursive link
                pe = new ParserElement("workingHours", 0, shiftNode);
                createSubTreeWorkingHours
                    (&XMLFile::doShiftWeekdayWorkingHours, pe,
                     &XMLFile::doShiftWeekdayWorkingHoursPost);
            }
        }

        // resourceList element
        pe = new ParserElement("resourceList", &XMLFile::doResourceList,
                               taskjugglerNode);
        ParserNode* resourceListNode = new ParserNode(pe);
        {
            pe = new ParserElement("resource", &XMLFile::doResource,
                                   resourceListNode);
            ParserNode* resourceNode = new ParserNode(pe);
            {
                resourceNode->add(pe, "resource");  // recursive link
                new ParserElement("flag", &XMLFile::doFlag, resourceNode);
                pe = new ParserElement("workingHours", 0, resourceNode);
                createSubTreeWorkingHours
                    (&XMLFile::doResourceWeekdayWorkingHours, pe,
                     &XMLFile::doResourceWeekdayWorkingHoursPost);
                createSubTreeVacationList(&XMLFile::doResourceVacation,
                                          resourceNode);
                createSubTreeTimeInterval("shiftSelection",
                                          &XMLFile::doShiftSelection,
                                          resourceNode);
                createSubTreeCustomAttribute(resourceNode);
            }
        }

        // accountList element
        pe = new ParserElement("accountList", &XMLFile::doAccountList,
                               taskjugglerNode);
        ParserNode* accountListNode = new ParserNode(pe);
        {
            pe = new ParserElement("account", &XMLFile::doAccount,
                                   accountListNode);
            ParserNode* accountNode = new ParserNode(pe);
            {
                accountNode->add(pe, "account"); // recursive link
                new ParserElement("flag", &XMLFile::doFlag, accountNode);
                createSubTreeCustomAttribute(accountNode);
            }
        }

        // taskList element
        pe = new ParserElement("taskList", &XMLFile::doTaskList,
                               taskjugglerNode);
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
            new ParserElement("flag", &XMLFile::doFlag, taskNode);
            pe = new ParserElement("depends", &XMLFile::doDepends, taskNode);
            ParserNode* dependencyGapScenarioNode = new ParserNode(pe);
            {
                new ParserElement("dependencyGapScenario",
                                  &XMLFile::doDependencyGapScenario,
                                  dependencyGapScenarioNode);
            }
            pe = new ParserElement("precedes", &XMLFile::doPrecedes, taskNode);
            dependencyGapScenarioNode = new ParserNode(pe);
            {
                new ParserElement("dependencyGapScenario",
                                  &XMLFile::doDependencyGapScenario,
                                  dependencyGapScenarioNode);
            }
            new ParserElement("note", &XMLFile::doNote, taskNode);
            createSubTreeCustomAttribute(taskNode);
        }

        // bookingList element
        pe = new ParserElement("bookingList", 0, taskjugglerNode);
        ParserNode* bookingListNode = new ParserNode(pe);
        {
            pe = new ParserElement("resourceBooking",
                                   &XMLFile::doResourceBooking,
                                   bookingListNode);
            ParserNode* resourceBookingNode = new ParserNode(pe);
            {
                createSubTreeTimeInterval("booking", &XMLFile::doBooking,
                                          resourceBookingNode,
                                          &XMLFile::doBookingPost);
            }
        }
    }
}

void
XMLFile::createSubTreeTimeInterval(const QString& id,
                                   ParserFunctionPtr preFunc,
                                   ParserNode* parentNode,
                                   ParserFunctionPtr postFunc)
{
    ParserElement* pe = new ParserElement(id, preFunc, parentNode, postFunc);
    ParserNode* timeIntervalNode = new ParserNode(pe);
    {
        new ParserElement("start", &XMLFile::doTimeIntervalStart,
                          timeIntervalNode);
        new ParserElement("end", &XMLFile::doTimeIntervalEnd,
                          timeIntervalNode);
    }
}

void
XMLFile::createSubTreeWorkingHours(ParserFunctionPtr func,
                                   ParserElement* parentEl,
                                   ParserFunctionPtr postFunc)
{
    ParserNode* workingHoursNode = new ParserNode(parentEl);
    {
        ParserElement* pe =
            new ParserElement("weekdayWorkingHours", func, workingHoursNode,
                              postFunc);
        ParserNode* weekdayWorkingHoursNode = new ParserNode(pe);
        {
            createSubTreeTimeInterval("timeInterval",
                                   &XMLFile::doTimeInterval,
                                   weekdayWorkingHoursNode);
        }
    }
}

void
XMLFile::createSubTreeVacationList(ParserFunctionPtr func,
                                   ParserNode* parentNode)
{
    ParserElement* pe = new ParserElement("vacationList", 0, parentNode);
    ParserNode* vacationListNode = new ParserNode(pe);
    {
        createSubTreeTimeInterval("vacation", func, vacationListNode);
    }
}

void
XMLFile::createSubTreeCustomAttribute(ParserNode* parentNode)
{
    ParserElement* pe =
        new ParserElement("customAttribute", &XMLFile::doCustomAttribute,
                          parentNode);
    ParserNode* customAttributeNode = new ParserNode(pe);
    {
        new ParserElement("textAttribute", &XMLFile::doTextAttribute,
                          customAttributeNode);
        new ParserElement("referenceAttribute",
                          &XMLFile::doReferenceAttribute,
                          customAttributeNode);
    }
}

bool
XMLFile::readDOM(const QString& file, const QString&, const QString&,
                 bool masterfile)
{
    if (masterfile)
    {
        project->setProgressBar(0, 100);
        masterFile = file;
    }

    gzFile zf;
    if (file == ".")
    {
        if ((zf = gzdopen(dup(STDIN_FILENO), "rb")) == NULL)
        {
            qWarning(i18n("Cannot open compressed STDIN for reading."));
            return FALSE;
        }
    }
    else
    {
        if ((zf = gzopen(file, "rb")) == NULL)
        {
            qWarning(i18n("Cannot open compressed file %1 for "
                          "reading.").arg(file));
            return FALSE;
        }
    }

    if (DEBUGLEVEL > 0)
        qWarning(i18n("Processing file \'%1\'").arg(file));

    QString buf;
    while (!gzeof(zf))
    {
        char cbuf[1024];
        gzgets(zf, cbuf, 1024);
        buf += cbuf;
    }
    int zError;
    if ((zError = gzclose(zf)) != 0)
    {
        qWarning(i18n("Cannot close compressed file %1: %2")
                 .arg(file).arg(gzerror(zf, &zError)));
        return FALSE;
    }

    doc = new QDomDocument(file);
    if (!doc->setContent(buf))
    {
        qWarning(i18n("Syntax error in XML file '%1'.").arg(file));
        return FALSE;
    }

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
    bool ret = TRUE;

    while (!n.isNull())
    {
        QDomElement el = n.toElement();
        if (!el.isNull())
        {
            const ParserElement* pEl = pn->getElement(el.tagName());
            if (!pEl)
            {
                qWarning(i18n("Unsupported XML element %1").arg(el.tagName()));
                ret = FALSE;
            }
            else
            {
                /* Create a copy of the current ptc. The node function may
                 * modify this copy to pass contextual information to the
                 * elements of this node. */
                ParserTreeContext ptcCopy = ptc;
                /* If a node pre function has been specified, call this function
                 * and pass it the ptc. */
                if (pEl->getPreFunc())
                    if (!((*this).*(pEl->getPreFunc()))(n, ptcCopy))
                        return FALSE;
                /* If sub-elements of this node have been defined in the
                 * parse tree, call this function again to process the
                 * sub-elements. */
                if (pEl->getNode())
                    if (!parseNode(pEl->getNode(), n.firstChild(), ptcCopy))
                        return FALSE;
                /* If a node post function has been specified, call this
                 * function and pass it the ptc. */
                if (pEl->getPostFunc())
                    if (!((*this).*(pEl->getPostFunc()))(n, ptcCopy))
                        return FALSE;
            }
        }
        n = n.nextSibling();
    }

    return ret;
}

bool
XMLFile::doTaskJuggler(QDomNode&, ParserTreeContext&)
{
    return TRUE;
}

bool
XMLFile::doProject(QDomNode& n, ParserTreeContext& ptc)
{
    QDomElement el = n.toElement();
    // mandatory attributes
    project->addId(el.attribute("id"));
    project->setName(el.attribute("name"));
    project->setVersion(el.attribute("version"));
    if (el.hasAttribute("timezone") && !el.attribute("timezone").isEmpty())
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

    /* Delete all default working hours since not all days have to be present
     * in the working hour specificiation. A missing day is a day off. */
    QPtrList<Interval> iv;
    for (int i = 0; i < 7; ++i)
        project->setWorkingHours(i, iv);

    ptc.setScenario(0);

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
    project->setEnd(n.toElement().text().toLong() - 1);
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
XMLFile::doCurrencyFormat(QDomNode& n, ParserTreeContext&)
{
    QDomElement el = n.toElement();

    project->setCurrencyFormat
        (RealFormat(el.attribute("signPrefix"), el.attribute("signSuffix"),
                    el.attribute("thousandSep"), el.attribute("fractionSep"),
                    el.attribute("fracDigits").toInt()));
    return TRUE;
}

bool
XMLFile::doExtend(QDomNode& n, ParserTreeContext& ptc)
{
    ptc.setExtendProperty(n.toElement().attribute("property"));
    return TRUE;
}

bool
XMLFile::doExtendAttribute(QDomNode& n, ParserTreeContext& ptc)
{
    QDomElement el = n.toElement();
    QString type = el.attribute("type");
    CustomAttributeType cat = CAT_Undefined;
    if (type == "text")
        cat = CAT_Text;
    else if (type == "reference")
        cat = CAT_Reference;
    CustomAttributeDefinition* ca =
        new CustomAttributeDefinition(el.attribute("name"), cat);
    if (!ca)
    {
        qWarning(i18n("Unknown custom attribute %1")
                 .arg(el.attribute("name")));
        return FALSE;
    }
    ca->setInherit(el.attribute("inherit").toInt());
    if (ptc.getExtendProperty() == "task")
        project->addTaskAttribute(el.attribute("id"), ca);
    else if (ptc.getExtendProperty() == "resource")
        project->addResourceAttribute(el.attribute("id"), ca);
    else if (ptc.getExtendProperty() == "account")
        project->addAccountAttribute(el.attribute("id"), ca);

    return TRUE;
}

bool
XMLFile::doProjectWeekdayWorkingHours(QDomNode& n, ParserTreeContext& ptc)
{
    QDomElement el = n.toElement();

    QPtrList<Interval>* wi = new QPtrList<Interval>;
    wi->setAutoDelete(true);
    ptc.setWorkingHours(wi);
    ptc.setWeekday(el.attribute("weekday").toInt());

    return true;
}

bool
XMLFile::doProjectWeekdayWorkingHoursPost(QDomNode&, ParserTreeContext& ptc)
{
    project->setWorkingHours(ptc.getWeekday(), *ptc.getWorkingHours());
    delete ptc.getWorkingHours();
    return true;
}

bool
XMLFile::doShiftWeekdayWorkingHours(QDomNode& n, ParserTreeContext& ptc)
{
    QDomElement el = n.toElement();

    QPtrList<Interval>* wi = new QPtrList<Interval>;
    wi->setAutoDelete(true);
    ptc.setWorkingHours(wi);
    ptc.setWeekday(el.attribute("weekday").toInt());

    return true;
}

bool
XMLFile::doShiftWeekdayWorkingHoursPost(QDomNode&, ParserTreeContext& ptc)
{
    ptc.getShift()->setWorkingHours(ptc.getWeekday(), *ptc.getWorkingHours());
    delete ptc.getWorkingHours();
    return true;
}

bool
XMLFile::doResourceWeekdayWorkingHours(QDomNode& n, ParserTreeContext& ptc)
{
    QDomElement el = n.toElement();

    QPtrList<Interval>* wi = new QPtrList<Interval>;
    wi->setAutoDelete(true);
    ptc.setWorkingHours(wi);
    ptc.setWeekday(el.attribute("weekday").toInt());

    return true;
}

bool
XMLFile::doResourceWeekdayWorkingHoursPost(QDomNode&, ParserTreeContext& ptc)
{
    ptc.getResource()->setWorkingHours
        (ptc.getWeekday(), *ptc.getWorkingHours());
    delete ptc.getWorkingHours();
    return true;
}

bool
XMLFile::doTimeInterval(QDomNode&, ParserTreeContext& ptc)
{
    Interval* iv = new Interval();
    ptc.getWorkingHours()->append(iv);
    ptc.setInterval(iv);

    return TRUE;
}

bool
XMLFile::doTimeIntervalStart(QDomNode& n, ParserTreeContext& ptc)
{
    ptc.getInterval()->setStart(n.toElement().text().toLong());
    return TRUE;
}

bool
XMLFile::doTimeIntervalEnd(QDomNode& n, ParserTreeContext& ptc)
{
    ptc.getInterval()->setEnd(n.toElement().text().toLong() - 1);
    return TRUE;
}

bool
XMLFile::doProjectVacation(QDomNode& n, ParserTreeContext& ptc)
{
    QDomElement el = n.toElement();
    VacationInterval* vi = new VacationInterval();
    vi->setName(el.attribute("name"));
    ptc.setVacationInterval(vi);
    project->addVacation(vi);
    return TRUE;
}

bool
XMLFile::doResourceVacation(QDomNode& n, ParserTreeContext& ptc)
{
    QDomElement el = n.toElement();
    Interval* vi = new Interval();
    ptc.setInterval(vi);
    ptc.getResource()->addVacation(vi);
    return TRUE;
}

bool
XMLFile::doCustomAttribute(QDomNode& n, ParserTreeContext& ptc)
{
    ptc.setExtendProperty(n.toElement().attribute("id"));
    return TRUE;
}

bool
XMLFile::doTextAttribute(QDomNode& n, ParserTreeContext& ptc)
{
    QDomElement el = n.toElement();
    TextAttribute* ta =
        new TextAttribute(el.attribute("text"));
    ptc.getCoreAttributes()->addCustomAttribute(ptc.getExtendProperty(), ta);
    return TRUE;
}

bool
XMLFile::doReferenceAttribute(QDomNode& n, ParserTreeContext& ptc)
{
    QDomElement el = n.toElement();
    ReferenceAttribute* ra =
        new ReferenceAttribute(el.attribute("url"), el.attribute("label"));
    ptc.getCoreAttributes()->addCustomAttribute(ptc.getExtendProperty(), ra);
    return TRUE;
}

bool
XMLFile::doShiftList(QDomNode&, ParserTreeContext& ptc)
{
    ptc.setShift(0);
    return TRUE;
}

bool
XMLFile::doShift(QDomNode& n, ParserTreeContext& ptc)
{
    QDomElement el = n.toElement();
    Shift* shift = new Shift(project, el.attribute("id"),
                             el.attribute("name"), ptc.getShift());
    ptc.setShift(shift);

    /* Delete all default working hours since not all days have to be present
     * in the working hour specificiation. A missing day is a day off. */
    QPtrList<Interval> iv;
    for (int i = 0; i < 7; ++i)
        shift->setWorkingHours(i, iv);

    return TRUE;
}

bool
XMLFile::doResourceList(QDomNode&, ParserTreeContext& ptc)
{
    ptc.setResource(0);
    return TRUE;
}

bool
XMLFile::doResource(QDomNode& n, ParserTreeContext& ptc)
{
    QDomElement el = n.toElement();
    Resource* r = new Resource(project, el.attribute("id"),
                               el.attribute("name"), ptc.getResource());

    /* Delete all default working hours since not all days have to be present
     * in the working hour specificiation. A missing day is a day off. */
    QPtrList<Interval> iv;
    for (int i = 0; i < 7; ++i)
        r->setWorkingHours(i, iv);

    ptc.setResource(r);

    return TRUE;
}

bool
XMLFile::doShiftSelection(QDomNode& n, ParserTreeContext& ptc)
{
    Interval* iv = new Interval();
    ptc.setInterval(iv);
    ShiftSelection* ss =
        new ShiftSelection
        (iv, project->getShift(n.toElement().attribute("shiftId")));
    ptc.getResource()->addShift(ss);

    return TRUE;
}

bool
XMLFile::doAccountList(QDomNode&, ParserTreeContext& ptc)
{
    ptc.setAccount(0);
    return TRUE;
}

bool
XMLFile::doAccount(QDomNode& n, ParserTreeContext& ptc)
{
    QDomElement el = n.toElement();
    Account* a = new Account(project, el.attribute("id"),
                             el.attribute("name"), ptc.getAccount(),
                             ptc.getAccount() ?
                             ptc.getAccount()->getAcctType() :
                             el.attribute("type") == "cost" ?
                             Cost : Revenue);
    ptc.setAccount(a);

    return TRUE;
}

bool
XMLFile::doTaskList(QDomNode&, ParserTreeContext& ptc)
{
    ptc.setTask(0);
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

    // Optional attributes
    if (!el.attribute("responsible").isEmpty())
        t->setResponsible(project->getResource(el.attribute("responsible")));
    if (!el.attribute("account").isEmpty())
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
    t->setComplete(sc, el.attribute("complete", "-1").toDouble());
    /* The scenario status will be ignored as it is computed after the file is
     * read in. */
    t->setStatusNote(sc, el.attribute("statusNote", ""));

    return TRUE;
}

bool
XMLFile::doTaskScenarioStart(QDomNode& n, ParserTreeContext& ptc)
{
    ptc.getTask()->setSpecifiedStart(ptc.getScenarioIndex(),
                                     n.toElement().text().toLong());
    return TRUE;
}

bool
XMLFile::doTaskScenarioEnd(QDomNode& n, ParserTreeContext& ptc)
{
    ptc.getTask()->setSpecifiedEnd(ptc.getScenarioIndex(),
                                   n.toElement().text().toLong() - 1);
    return TRUE;
}

bool
XMLFile::doTaskScenarioMaxEnd(QDomNode& n, ParserTreeContext& ptc)
{
    ptc.getTask()->setMaxEnd(ptc.getScenarioIndex(),
                             n.toElement().text().toLong() - 1);
    return TRUE;
}

bool
XMLFile::doTaskScenarioMinEnd(QDomNode& n, ParserTreeContext& ptc)
{
    ptc.getTask()->setMinEnd(ptc.getScenarioIndex(),
                             n.toElement().text().toLong() - 1);
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

bool
XMLFile::doDepends(QDomNode& n, ParserTreeContext& ptc)
{
    ptc.setTaskDependency
        (ptc.getTask()->addDepends(n.toElement().text()));
    return TRUE;
}

bool
XMLFile::doPrecedes(QDomNode& n, ParserTreeContext& ptc)
{
    ptc.setTaskDependency
        (ptc.getTask()->addPrecedes(n.toElement().text()));
    return TRUE;
}

bool
XMLFile::doDependencyGapScenario(QDomNode& n, ParserTreeContext& ptc)
{
    QDomElement el = n.toElement();
    int sc = project->getScenarioIndex(el.attribute("scenarioId")) - 1;
    ptc.getTaskDependency()->setGapDuration
        (sc, el.attribute("gapDuration", "0").toLong());
    ptc.getTaskDependency()->setGapLength
        (sc, el.attribute("gapLength", "0").toLong());

    return TRUE;
}

bool
XMLFile::doNote(QDomNode& n, ParserTreeContext& ptc)
{
    ptc.getTask()->setNote(n.toElement().text());
    return TRUE;
}

bool
XMLFile::doResourceBooking(QDomNode& n, ParserTreeContext& ptc)
{
    QDomElement el = n.toElement();
    Resource* resource = project->getResource(el.attribute("resourceId"));
    if (!resource)
    {
        qWarning(i18n("Booking for unknown resource %1")
                 .arg(el.attribute("resourceId")));
        return FALSE;
    }
    ptc.setResource(resource);
    int sc = project->getScenarioIndex(el.attribute("scenarioId")) - 1;
    if (sc < 0)
    {
        qWarning(i18n("Booking for unknown scenario %1")
                 .arg(el.attribute("scenarioId")));
        return FALSE;
    }
    ptc.setScenarioIndex(sc);
    return TRUE;
}

bool
XMLFile::doBooking(QDomNode&, ParserTreeContext& ptc)
{
    Interval* iv = new Interval();
    ptc.setInterval(iv);
    return TRUE;
}

bool
XMLFile::doBookingPost(QDomNode& n, ParserTreeContext& ptc)
{
    Task* t = project->getTask(n.toElement().attribute("taskId"));
    if (!t)
    {
        qWarning(i18n("Booking for unknown task %1")
                 .arg(n.toElement().attribute("taskId")));
        return FALSE;
    }
    ptc.getResource()->addBooking(ptc.getScenarioIndex(),
                                  new Booking(ptc.getInterval(), t));

    return TRUE;
}

