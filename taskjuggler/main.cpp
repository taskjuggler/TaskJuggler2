/*
 * main.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * As a special exception, you have permission to link this program with
 * the Qt3 library and distribute executables, as long as you follow the
 * requirements of the GNU GPL version 2 in regard to all of the software
 * in the executable aside from Qt3.
 *
 * $Id$
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <qapplication.h>
#include <qglobal.h>
#include <qfile.h>

#include "TjMessageHandler.h"
#include "tjlib-internal.h"
#include "taskjuggler.h"
#include "debug.h"
#include "Project.h"
#include "ProjectFile.h"
#include "ScenarioList.h"
#include "Scenario.h"
#include "Optimizer.h"
#include "OptimizerRun.h"
#include "ParserElement.h"
#include "kotrus.h"

void
banner()
{
    qWarning(i18n("TaskJuggler v%1 - A Project Management Software")
             .arg(VERSION));
}

void
copyright()
{
    qWarning
        (i18n(
              "\nCopyright (c) 2001, 2002, 2003, 2004, 2005, 2006 \n"
              "by Chris Schlaeger <cs@kde.org> and Klaas Freitag <freitag@suse.de>\n\n"
              "This program is free software; you can redistribute it and/or\n"
              "modify it under the terms of version 2 of the GNU General\n"
              "Public License as published by the Free Software Foundation.\n\n"
              "For more information about TaskJuggler see \n%1\n")
         .arg(TJURL));
}

void
usage(QApplication& a)
{
    qWarning
        (i18n(
              "TaskJuggler must be called with at least one file that\n"
              "contains the project description and the report definitions.\n"
              "\n"
              "Usage: %1 [options] <filename1> [<filename2> ...]")
             .arg(a.argv()[0]));
    qWarning
        (i18n(
              "   --help               - print this message\n"
              "   --version            - print the version and copyright info\n"
              "   -v                   - same as '--version'\n"
              "   -s                   - stop after syntax check\n"
              "   -M                   - output include dependencies for\n"
              "                          make utilities\n"
              "   --makefile <file>    - generate include dependencies for make\n"
              "                          utilities into the specified file\n"
              "   --maxerrors N        - stop after N errors. If N is 0 show all\n"
              "                          errors\n"
              "   --warnerrors         - warnings are treated as errors\n"
              "   --nodepcheck         - don't search for dependency loops\n"
              "   --debug N            - print debug output, N must be between\n"
              "                          0 and 40, the higher N the more output\n"
              "                          is printed\n"
              "   --dbmode N           - activate debug mode only for certain\n"
              "                          parts of the code\n"
              "   --updatedb           - update the Kotrus database with the\n"
              "                          new resource usage information\n"));
    qWarning
        (i18n(
              "To report bugs please follow the instructions in the "
              "manual.\n"));
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv, false);

    int debugLevel = 0;
    int debugMode = -1;
    int maxErrors = 10;
    bool updateKotrusDB = false;
    bool checkOnlySyntax = false;
    bool generateMakeDepList = false;
    bool noDepCheck = false;
    bool showHelp = false;
    bool showCopyright = false;
    bool terminateProgram = false;
    bool warningAsErrors = false;
    QString makeDepFile;

    int i;
    for (i = 1 ; i < a.argc() && a.argv()[i][0] == '-'; i++)
        if (strcmp(a.argv()[i], "--help") == 0)
            showCopyright = showHelp = true;
        else if (strcmp(a.argv()[i], "--debug") == 0)
        {
            if (i + 1 >= a.argc())
            {
                qWarning(i18n("--debug needs numerical argument"));
                showCopyright = showHelp = terminateProgram = true;
            }
            debugLevel = QString(a.argv()[++i]).toInt();
        }
        else if (strcmp(a.argv()[i], "--dbmode") == 0)
        {
            if (i + 1 >= a.argc())
            {
                qWarning(i18n("--dbmode needs numerical argument"));
                showCopyright = showHelp = terminateProgram = true;
            }
            debugMode = QString(a.argv()[++i]).toInt();
        }
        else if (strcmp(a.argv()[i], "--makefile") == 0)
        {
            if (i + 1 >= a.argc())
            {
                qWarning(i18n("--makefile needs filename argument"));
                showCopyright = showHelp = terminateProgram = true;
            }
            makeDepFile = a.argv()[++i];
            generateMakeDepList = true;
        }
        else if (strcmp(a.argv()[i], "--maxerrors") == 0)
        {
            bool ok;
            if (i + 1 >= a.argc() ||
                (maxErrors = QString(a.argv()[++i]).toInt(&ok), !ok))
            {
                qWarning(i18n("--maxerrors needs a numerical argument"));
                showCopyright = showHelp = terminateProgram = true;
            }
        }
        else if (strcmp(a.argv()[i], "--version") == 0 ||
                 strcmp(a.argv()[i], "-v") == 0)
            showCopyright = true;
        else if (strcmp(a.argv()[i], "-s") == 0)
            checkOnlySyntax = true;
        else if (strcmp(a.argv()[i], "-M") == 0)
            generateMakeDepList = true;
        else if (strcmp(a.argv()[i], "--nodepcheck") == 0)
            noDepCheck = true;
        else if (strcmp(a.argv()[i], "--updatedb") == 0)
            updateKotrusDB = true;
        else if (strcmp(a.argv()[i], "--warnerror") == 0)
            warningAsErrors = true;
        else
            showCopyright = showHelp = terminateProgram = true;

    if (a.argc() - i < 1)
    {
        if (!showCopyright && !showHelp)
            showCopyright = showHelp = true;
        terminateProgram = true;
    }

    if (showCopyright)
    {
        banner();
        copyright();
    }
    else if (debugLevel > 0)
        banner();

    if (showHelp)
        usage(a);
    if (terminateProgram)
        exit(EXIT_FAILURE);

    DebugCtrl.setDebugLevel(debugLevel);
    DebugCtrl.setDebugMode(debugMode);

    bool parseErrors = false;

    char cwd[1024];
    if (getcwd(cwd, 1023) == 0)
        qFatal("main(): getcwd() failed");
    Project p;
    p.setMaxErrors(maxErrors);
    bool first = true;
    for ( ; i < argc; i++)
    {
        QString fileName = a.argv()[i];
        if (fileName.right(4) == ".tjx")
        {
            XMLFile* xf = new XMLFile(&p);
            if (!xf->readDOM(fileName, QFile::decodeName(cwd) + "/", "", true))
                exit(EXIT_FAILURE);
            parseErrors |= !xf->parse();
            delete xf;
        }
        else
        {
            if (fileName != "." &&
                fileName.right(4) != ".tjp" &&
                fileName.right(4) != ".tji")
            {
                qWarning(i18n("WARNING: %1 has an unsupported file extension. "
                              "Please use '.tjp' for toplevel files, '.tji' "
                              "for included files and '.tjx' for TaskJuggler "
                              "XML files.")
                         .arg(fileName));
                // parseErrors = true;
            }
            ProjectFile* pf = new ProjectFile(&p);
            if (!pf->open(a.argv()[i], QFile::decodeName(cwd) + "/", "", true))
                exit(EXIT_FAILURE);
            parseErrors |= !pf->parse();
            if (generateMakeDepList)
                pf->generateMakeDepList(makeDepFile, !first);
            if (first)
                first = false;
            delete pf;
        }
    }

    p.readKotrus();

    bool schedulingErrors = false;
    int errors = 0;
    int warnings = 0;
    bool logicalErrors = !p.pass2(noDepCheck, schedulingErrors, errors,
                                  warnings);
    if (warningAsErrors && warnings > 0)
        logicalErrors = true;

    int oldWarnings = warnings;
    if (!schedulingErrors && !(checkOnlySyntax || generateMakeDepList))
    {
        schedulingErrors = !p.scheduleAllScenarios(errors, warnings);
        if (warningAsErrors && warnings != oldWarnings)
            schedulingErrors = true;
        if (updateKotrusDB)
            if (parseErrors || logicalErrors || schedulingErrors)
                qWarning("Due to errors the Kotrus DB will NOT be "
                         "updated.");
            else
                p.updateKotrus();

        p.generateReports();
    }

    return (parseErrors || logicalErrors || schedulingErrors ?
            EXIT_FAILURE : EXIT_SUCCESS);
}
