/*
 * main.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <qapplication.h>
#include <qglobal.h>

#include "TjMessageHandler.h"
#include "taskjuggler.h"
#include "debug.h"
#include "Project.h"
#include "ProjectFile.h"
#include "kotrus.h"

TjMessageHandler TJMH(TRUE);

void
copyright()
{
    qWarning("TaskJuggler v%s - A Project Management Software\n"
             "Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>\n"
             "and Klaas Freitag <freitag@suse.de>\n\n"
             "This program is free software; you can redistribute it and/or\n"
             "modify it under the terms of version 2 of the GNU General\n"
             "Public License as published by the Free Software Foundation.\n\n"
             "For more information about TaskJuggler see \n%s\n",
             VERSION, TJURL);
}

void 
usage(QApplication& a)
{
    qWarning("Usage: %s [options] <filename1> [<filename2> ...]", a.argv()[0]);
    qWarning(
        "   --help               - print this message\n"
        "   --version            - print the version and copyright info\n"
        "   -v                   - same as '--version'\n"
        "   -s                   - stop after syntax check\n"
        "   --nodepcheck         - don't search for dependency loops\n"
        "   --debug N            - print debug output, N must be between\n"
        "                          0 and 4, the higher N the more output\n"
        "                          is printed\n"
        "   --dbmode N           - activate debug mode only for certain\n"
        "                          parts of the code\n"
        "   --updatedb           - update the Kotrus database with the\n"
        "                          new resource usage information\n");
    qWarning("To report bugs please follow the instructions in the manual\n"
             "and send the information to the taskjuggler developer mailing\n"
             "list at taskjuggler-devel@suse.de\n");
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv, false);

    int debugLevel = 0;
    int debugMode = -1;
    bool updateKotrusDB = FALSE;
    bool checkOnlySyntax = FALSE;
    bool noDepCheck = FALSE;
    bool showHelp = FALSE;
    bool showCopyright = FALSE;
    bool terminateProgram = FALSE;

    int i;
    for (i = 1 ; i < a.argc() && a.argv()[i][0] == '-'; i++)
        if (strcmp(a.argv()[i], "--help") == 0)
            showCopyright = showHelp = TRUE;
        else if (strcmp(a.argv()[i], "--debug") == 0)
        {
            if (i + 1 >= a.argc())
            {
                qWarning("--debug needs numerical argument");
                showCopyright = showHelp = terminateProgram = 1;
            }
            debugLevel = QString(a.argv()[++i]).toInt();
        }
        else if (strcmp(a.argv()[i], "--dbmode") == 0)
        {
            if (i + 1 >= a.argc())
            {
                qWarning("--dbmode needs numerical argument");
                showCopyright = showHelp = terminateProgram = 1;
            }
            debugMode = QString(a.argv()[++i]).toInt();
        }
        else if (strcmp(a.argv()[i], "--version") == 0 ||
                 strcmp(a.argv()[i], "-v") == 0)
            showCopyright = TRUE;
        else if (strcmp(a.argv()[i], "-s") == 0)
            checkOnlySyntax = TRUE;
        else if (strcmp(a.argv()[i], "--nodepcheck") == 0)
            noDepCheck = TRUE;
        else if (strcmp(a.argv()[i], "--updatedb") == 0)
            updateKotrusDB = TRUE;
        else
            showCopyright = showHelp = terminateProgram = TRUE;
    
    if (a.argc() - i < 1)
    {
        if (!showCopyright && !showHelp)
            showCopyright = showHelp = TRUE;
        terminateProgram = TRUE;
    }

    if (showCopyright)
        copyright();
    if (showHelp)
        usage(a);
    if (terminateProgram)
        exit(1);

    DebugCtrl.setDebugLevel(debugLevel);    
    DebugCtrl.setDebugMode(debugMode);

    bool parseErrors = FALSE;

    char cwd[1024];
    if (getcwd(cwd, 1023) == 0)
        qFatal("main(): getcwd() failed");
    if (debugLevel >= 1)
        qWarning("Reading input files...");
    Project p;
    for ( ; i < argc; i++)
    {
        ProjectFile* pf = new ProjectFile(&p);
        if (!pf->open(a.argv()[i], QString(cwd) + "/", ""))
            return (-1);
        parseErrors = !pf->parse();
        delete pf;
    }

    p.readKotrus();

    bool logicalErrors = !p.pass2(noDepCheck);
    bool schedulingErrors = FALSE;

    if (!checkOnlySyntax)
    {
        schedulingErrors = p.getAllocationErrors() || !p.scheduleAllScenarios();
        if (updateKotrusDB)
            if (parseErrors || logicalErrors || schedulingErrors)
                qWarning("Due to errors the Kotrus DB will NOT be "
                         "updated.");
            else
                p.updateKotrus();

        p.generateReports();
    }

    return (parseErrors || logicalErrors || schedulingErrors ? -1 : 0);
}
