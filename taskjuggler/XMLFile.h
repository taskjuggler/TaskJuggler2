/*
 * XMLFile.h - TaskJuggler
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

#ifndef _XMLFile_h_
#define _XMLFile_h_

#include <time.h>

#include "ParserElement.h"

class Project;
class QDomDocument;
class QDomNode;
class ParserNode;
class ParserTreeContext;

/**
 * @short File Parser for project files.
 * @author Chris Schlaeger <cs@kde.org>
 */
class XMLFile
{
public:
    /**
     * A XMLFile cannot exist without a project. So the constructor needs
     * to know what Project object to fill, when it parses the project files.
     */
    XMLFile(Project* p);
    ~XMLFile();

    /**
     * The files XML files needs to be opened before the parser can be
     * started.
     * @param file The file name of the file to start with.
     * @param parentPath The path of the file that included this file. This
     * feature is for internal use only. It's not part of the public API.
     * @param taskPrefix The ID prefix of the parent task. This is needed when
     * the tasks of the project file should be read as a sub-task of an
     * already existing task.
     * @param This flag must be true if a top-level file should be processed.
     * This is a file, that is not included by any other file.
     */
    bool readDOM(const QString& file, const QString& parentPath,
                 const QString& taskPrefix, bool masterfile = false);

    /**
     * Calling the parse function will start the processing of the DOM tree
     * It will automatically read all include files as well. The
     * collected data is stored into the Project object.
     */
    bool parse();

private:
    XMLFile() {};   // don't use

    void createParseTree();
    void createSubTreeTimeInterval(const QString& id, ParserFunctionPtr preFunc,
                                   ParserNode* parentNode,
                                   ParserFunctionPtr postFunc = 0);
    void createSubTreeWorkingHours(ParserFunctionPtr func,
                                   ParserElement* parentEl,
                                   ParserFunctionPtr postFunc);
    void createSubTreeVacationList(ParserFunctionPtr func,
                                   ParserNode* parentNode);
    void createSubTreeCustomAttribute(ParserNode* parentNode);

    bool parseNode(const ParserNode* pn, QDomNode n, ParserTreeContext ptc);

    bool doTaskJuggler(QDomNode& n, ParserTreeContext& ptc);
    bool doProject(QDomNode& n, ParserTreeContext& ptc);
    bool doProjectStart(QDomNode& n, ParserTreeContext& ptc);
    bool doProjectEnd(QDomNode& n, ParserTreeContext& ptc);
    bool doProjectNow(QDomNode& n, ParserTreeContext& ptc);
    bool doCurrencyFormat(QDomNode& n, ParserTreeContext& ptc);
    bool doScenario(QDomNode& n, ParserTreeContext& ptc);
    bool doExtend(QDomNode& n, ParserTreeContext& ptc);
    bool doExtendAttribute(QDomNode& n, ParserTreeContext& ptc);
    bool doProjectWeekdayWorkingHours(QDomNode& n, ParserTreeContext& ptc);
    bool doProjectWeekdayWorkingHoursPost(QDomNode& n, ParserTreeContext& ptc);
    bool doShiftWeekdayWorkingHours(QDomNode& n, ParserTreeContext& ptc);
    bool doShiftWeekdayWorkingHoursPost(QDomNode& n, ParserTreeContext& ptc);
    bool doResourceWeekdayWorkingHours(QDomNode& n, ParserTreeContext& ptc);
    bool doResourceWeekdayWorkingHoursPost(QDomNode& n, ParserTreeContext& ptc);
    bool doTimeInterval(QDomNode& n, ParserTreeContext& ptc);
    bool doTimeIntervalStart(QDomNode& n, ParserTreeContext& ptc);
    bool doTimeIntervalEnd(QDomNode& n, ParserTreeContext& ptc);
    bool doProjectVacation(QDomNode& n, ParserTreeContext& ptc);
    bool doResourceVacation(QDomNode& n, ParserTreeContext& ptc);
    bool doVacationStart(QDomNode& n, ParserTreeContext& ptc);
    bool doVacationEnd(QDomNode& n, ParserTreeContext& ptc);
    bool doCustomAttribute(QDomNode& n, ParserTreeContext& ptc);
    bool doTextAttribute(QDomNode& n, ParserTreeContext& ptc);
    bool doReferenceAttribute(QDomNode& n, ParserTreeContext& ptc);
    bool doShiftList(QDomNode& n, ParserTreeContext& ptc);
    bool doShift(QDomNode& n, ParserTreeContext& ptc);
    bool doResourceList(QDomNode& n, ParserTreeContext& ptc);
    bool doResource(QDomNode& n, ParserTreeContext& ptc);
    bool doShiftSelection(QDomNode& n, ParserTreeContext& ptc);
    bool doAccountList(QDomNode& n, ParserTreeContext& ptc);
    bool doAccount(QDomNode& n, ParserTreeContext& ptc);
    bool doTaskList(QDomNode& n, ParserTreeContext& ptc);
    bool doTask(QDomNode& n, ParserTreeContext& ptc);
    bool doTaskScenario(QDomNode& n, ParserTreeContext& ptc);
    bool doTaskScenarioStart(QDomNode& n, ParserTreeContext& ptc);
    bool doTaskScenarioEnd(QDomNode& n, ParserTreeContext& ptc);
    bool doTaskScenarioMaxEnd(QDomNode& n, ParserTreeContext& ptc);
    bool doTaskScenarioMinEnd(QDomNode& n, ParserTreeContext& ptc);
    bool doTaskScenarioMaxStart(QDomNode& n, ParserTreeContext& ptc);
    bool doTaskScenarioMinStart(QDomNode& n, ParserTreeContext& ptc);
    bool doAllocate(QDomNode& n, ParserTreeContext& ptc);
    bool doCandidate(QDomNode& n, ParserTreeContext& ptc);
    bool doDepends(QDomNode& n, ParserTreeContext& ptc);
    bool doPrecedes(QDomNode& n, ParserTreeContext& ptc);
    bool doDependencyGapScenario(QDomNode& n, ParserTreeContext& ptc);
    bool doNote(QDomNode& n, ParserTreeContext& ptc);
    bool doFlag(QDomNode& n, ParserTreeContext& ptc);
    bool doResourceBooking(QDomNode& n, ParserTreeContext& ptc);
    bool doBooking(QDomNode& n, ParserTreeContext& ptc);
    bool doBookingPost(QDomNode& n, ParserTreeContext& ptc);

    QString masterFile;
    Project* project;
    QDomDocument* doc;

    static ParserNode* parserRootNode;
};
#endif

