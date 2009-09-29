/*
 * SVGReport.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _SVGReport_h_
#define _SVGReport_h_

#include "Report.h"
#include "Scenario.h"

class ExternalProject
{
public:
    ExternalProject(QString pf, QString sc):projectFile(pf),scenario(sc),project(0),sc(0) { }
    QString projectFile;
    QString scenario;
    Project* project;
    Scenario* sc;
};

class AllScenario {
public:
    AllScenario():project(0),sc(0) { }
    AllScenario(Project* pr, Scenario* scnr):project(pr),sc(scnr) { }
    Project* project;
    Scenario* sc;
    AllScenario operator=(const AllScenario& op) { project = op.project; sc = op.sc; }
};

/**
 * @short Stores all information about an SVG report.
 */
class SVGReport : public Report
{
public:
    SVGReport(Project* p, const QString& f, const QString& df, int dl);

    virtual ~SVGReport();

    void setCaption(const QString& s) { caption = s; }
    const QString& getCaption() const { return caption; }

    virtual void inheritValues();

    void addProjectFile(const QString pr, const QString sc) { externalProjects.append(new ExternalProject(pr, sc)); }
protected:
    bool parseExternalProject();
    QPtrList<ExternalProject> externalProjects;
    QPtrList<AllScenario> dateSortedAllScenarios;

private:
    QString caption;
};

#endif
