/*
 * SVGReport.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <string>

#include <config.h>
#include <qdir.h>

#ifdef HAVE_KDE

#include "SVGReport.h"

#include "TjMessageHandler.h"
#include "tjlib-internal.h"
#include "Project.h"
#include "ProjectFile.h"
#include "Scenario.h"
#include "debug.h"

// Older versions of KDE do not have this macro
#ifndef KDE_IS_VERSION
#define KDE_IS_VERSION(a,b,c) 0
#endif

SVGReport::SVGReport(Project* p, const QString& file, const QString& defFile,
                       int dl) :
    Report(p, file, defFile, dl),
    externalProjects()
{
    // By default, use all scenarios.
    unsigned int i = 0;
    for (ScenarioListIterator sli(project->getScenarioIterator()); *sli ; ++sli, ++i)
    {
        if ((*sli)->getEnabled())
            scenarios.append(i);
    }

    taskSortCriteria[0] = CoreAttributesList::NameUp;

}

SVGReport::~SVGReport()
{
    ExternalProject *externalProject = 0;
    for (QPtrListStdIterator<ExternalProject>  it = externalProjects.begin();
         it != externalProjects.end(); ++it)
    {
        if (*it)
        {
            externalProject = *it;
            if (externalProject->project) delete externalProject->project;
            delete externalProject;
        }
    }
}

void SVGReport::inheritValues()
{
    Report::inheritValues();

    SVGReport* parent = dynamic_cast<SVGReport*>(getParentReport());

    if (parent)
    {
        setCaption(parent->getCaption());

        ExternalProject *externalProject = 0;
        for (QPtrListStdIterator<ExternalProject>  it = parent->externalProjects.begin();
            it != parent->externalProjects.end(); ++it)
        {
            externalProject = *it;
            externalProjects.append(new ExternalProject(externalProject->projectFile,externalProject->scenario));
        }
    }
}

bool SVGReport::parseExternalProject()
{
    ExternalProject *externalProject = 0;
    for (QPtrListStdIterator<ExternalProject>  it = externalProjects.begin();
         it != externalProjects.end(); ++it)
    {

        externalProject = *it;
        if (externalProject && externalProject->projectFile != "")
        {
            QString mainFileName = QDir(this->getProject()->getSourceFiles()[0]).absPath();
            QString dir = mainFileName.left(mainFileName.length() - QFileInfo(mainFileName).fileName().length());
            QDir d( dir, externalProject->projectFile);

            // Because the QDir is not flexible in term of wildcarding files and path names,
            // the 'find' shell function is preferred, giving users full flexibility to define files.

            char * tmpfimename;

            tmpfimename = tmpnam (NULL);
            QString tjpFileName;
            if (dir == externalProject->projectFile.left(dir.length()))
                tjpFileName = externalProject->projectFile;
            else
                tjpFileName = dir + externalProject->projectFile;

            QString shellCmd = "find " + tjpFileName  + " > " + tmpfimename;
            system(shellCmd);

            std::string line;
            std::ifstream myfile (tmpfimename);
            if (!myfile.is_open())
            {
                errorMessage(i18n("Cannot open file temporary file '%1' for reading.").arg(tmpfimename));
                return false;
            }
            bool first = true;
            while (!myfile.eof() )
            {
                getline (myfile,line);
                if (line.length() == 0) continue;
                QString tjpFile = line;

                if (!first)
                {
                    externalProject = new ExternalProject(tjpFile, externalProject->scenario);
                    externalProjects.append(externalProject);
                    externalProject->projectFile = tjpFile;
                    continue;  // Will be processed later in the main for loop
                }
                first = false;
                externalProject->projectFile = tjpFile;
                externalProject->project = new Project();
                externalProject->project->setMaxErrors(this->getProject()->getMaxErrors());
                ProjectFile pf(externalProject->project);
                if (!pf.open(tjpFile, dir, "", false))
                {
                    errorMessage(i18n("Cannot open file '%1' in '%2'").arg(externalProject->projectFile).arg(dir));
                    errorMessage(i18n("Cannot open file '%1'").arg(tjpFile));
                    return false;
                }

                pf.parse();

                externalProject->project->pass2(false);

                if (TJMH.getErrors() == 0)
                {
                    externalProject->project->scheduleAllScenarios();
                }
                else
                    return false;

                if (externalProject->scenario != "*")
                {
                    bool found = false;
                    for (ScenarioListIterator sli(externalProject->project->getScenarioIterator()); *sli ; ++sli)
                    {
                        if ((*sli)->getId() == externalProject->scenario
                            || (externalProject->scenario == "" && (*sli)->getEnabled()))
                        {
                            if (!(*sli)->getEnabled()) {
                                errorMessage(i18n("Scenario '%1' in file '%2' is not enabled.")
                                    .arg(externalProject->scenario)
                                    .arg(externalProject->projectFile));
                                return false;
                            }
                            if (externalProject->scenario == "") externalProject->scenario = (*sli)->getId();
                            externalProject->sc = (*sli);

                            if ((*sli)->getDate() == 0) {
                                (*sli)->setDate(externalProject->project->getNow());
                            }

                            dateSortedAllScenarios.append(new AllScenario(externalProject->project, externalProject->sc));
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        errorMessage(i18n("Scenario '%1' not found in file '%2'.")
                            .arg(externalProject->scenario)
                            .arg(externalProject->projectFile));
                        return false;
                    }
                }
                else
                {
                    bool first = true;
                    for (ScenarioListIterator sli(externalProject->project->getScenarioIterator()); *sli ; ++sli)
                    {
                        if (first) {
                            first = false;
                            externalProject->scenario = (*sli)->getId();
                            externalProject->sc = (*sli);
                        }
                        else
                        {
                            ExternalProject* ep = new ExternalProject(externalProject->projectFile, (*sli)->getId());
                            ep->project = externalProject->project;
                            ep->sc = (*sli);
                            externalProjects.append(ep);
                        }
                        if ((*sli)->getDate() == 0) {
                            (*sli)->setDate(externalProject->project->getNow());
                        }
                        dateSortedAllScenarios.append(new AllScenario(externalProject->project, (*sli)));
                    }
                }
            }
            myfile.close();
            QFile(tmpfimename).remove();
        }
    }

    // Add main projec scenarios in dateSortedAllScenarios
    for (QValueList<int>::const_iterator it = scenarios.begin(); it != scenarios.end(); ++it)
    {
        dateSortedAllScenarios.append(new AllScenario(project, getProject()->getScenario(*it)));
    }

    // Then sort scenarios by date
    AllScenario buf;
    for (QPtrListStdIterator<AllScenario>  idsas = dateSortedAllScenarios.begin();
         idsas != dateSortedAllScenarios.end(); ++idsas)
    {
        QPtrListStdIterator<AllScenario>  idsas2 = idsas;
        idsas2++;
        for (; idsas2 != dateSortedAllScenarios.end(); ++idsas2)
        {
            if ((*idsas2)->sc->getDate() < (*idsas)->sc->getDate())
            {
                buf = *(*idsas);
                *(*idsas) = *(*idsas2);
                *(*idsas2) = buf;
                break;
            }
        }
    }

    return true;
}

#endif