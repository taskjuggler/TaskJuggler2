/*
 * XMLFile.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
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

#include <qstring.h>

#include "ParserElement.h"

class Project;
class QDomDocument;
class QDomNode;
class ParserNode;
class ParserTreeContext;

/**
 * @short File Parser for project files.
 * @author Chris Schlaeger <cs@suse.de>
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
                 const QString& taskPrefix, bool masterfile = FALSE);
   
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
                                   ParserElement* parentEl);
    void createSubTreeVacationList(ParserFunctionPtr func,
                                   ParserNode* parentNode);
    void createSubTreeCustomAttribute(ParserNode* parentNode);

    bool parseNode(const ParserNode* pn, QDomNode n, ParserTreeContext ptc);

    bool doTaskJuggler(QDomNode& n, ParserTreeContext& n);
    bool doProject(QDomNode& n, ParserTreeContext& n);
    bool doProjectStart(QDomNode& n, ParserTreeContext& n);
    bool doProjectEnd(QDomNode& n, ParserTreeContext& n);
    bool doProjectNow(QDomNode& n, ParserTreeContext& n);
    bool doCurrencyFormat(QDomNode& n, ParserTreeContext& n);
    bool doScenario(QDomNode& n, ParserTreeContext& n);
    bool doExtend(QDomNode& n, ParserTreeContext& n);
    bool doExtendAttribute(QDomNode& n, ParserTreeContext& n);
    bool doProjectWeekdayWorkingHours(QDomNode& n, ParserTreeContext& n);
    bool doShiftWeekdayWorkingHours(QDomNode& n, ParserTreeContext& n);
    bool doResourceWeekdayWorkingHours(QDomNode& n, ParserTreeContext& n);
    bool doTimeInterval(QDomNode& n, ParserTreeContext& n);
    bool doTimeIntervalStart(QDomNode& n, ParserTreeContext& n);
    bool doTimeIntervalEnd(QDomNode& n, ParserTreeContext& n);
    bool doProjectVacation(QDomNode& n, ParserTreeContext& n);
    bool doResourceVacation(QDomNode& n, ParserTreeContext& n);
    bool doVacationStart(QDomNode& n, ParserTreeContext& n);
    bool doVacationEnd(QDomNode& n, ParserTreeContext& n);
    bool doCustomAttribute(QDomNode& n, ParserTreeContext& n);
    bool doTextAttribute(QDomNode& n, ParserTreeContext& n);
    bool doReferenceAttribute(QDomNode& n, ParserTreeContext& n);
    bool doShiftList(QDomNode& n, ParserTreeContext& n);
    bool doShift(QDomNode& n, ParserTreeContext& n);
    bool doResourceList(QDomNode& n, ParserTreeContext& n);
    bool doResource(QDomNode& n, ParserTreeContext& n);
    bool doShiftSelection(QDomNode& n, ParserTreeContext& n);
    bool doAccountList(QDomNode& n, ParserTreeContext& n);
    bool doAccount(QDomNode& n, ParserTreeContext& n);
    bool doTaskList(QDomNode& n, ParserTreeContext& n);
    bool doTask(QDomNode& n, ParserTreeContext& n);
    bool doTaskScenario(QDomNode& n, ParserTreeContext& n);
    bool doTaskScenarioStart(QDomNode& n, ParserTreeContext& n);
    bool doTaskScenarioEnd(QDomNode& n, ParserTreeContext& n);
    bool doTaskScenarioMaxEnd(QDomNode& n, ParserTreeContext& n);
    bool doTaskScenarioMinEnd(QDomNode& n, ParserTreeContext& n);
    bool doTaskScenarioMaxStart(QDomNode& n, ParserTreeContext& n);
    bool doTaskScenarioMinStart(QDomNode& n, ParserTreeContext& n);
    bool doAllocate(QDomNode& n, ParserTreeContext& n);
    bool doCandidate(QDomNode& n, ParserTreeContext& n);
    bool doDepends(QDomNode& n, ParserTreeContext& n);
    bool doPrecedes(QDomNode& n, ParserTreeContext& n);
    bool doFlag(QDomNode& n, ParserTreeContext& n);
    bool doResourceBooking(QDomNode& n, ParserTreeContext& n);
    bool doBooking(QDomNode& n, ParserTreeContext& n);
    bool doBookingPost(QDomNode& n, ParserTreeContext& n);

    QString masterFile;
    Project* project;
    QDomDocument* doc;

    static ParserNode* parserRootNode;
};
#endif

