/*
 * main.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>

#include <qapplication.h>
#include <qglobal.h>

#include "Project.h"
#include "ProjectFile.h"
#include "kotrus.h"

void
copyright()
{
	qWarning("TaskJuggler v%s - A Project Management Software\n"
			 "Copyright (c) 2001, 2002 by Chris Schlaeger <cs@suse.de> and\n"
			 "                         Klaas Freitag <freitag@suse.de>\n\n"
			 "This program is free software; you can redistribute it and/or\n"
			 "modify it under the terms of version 2 of the GNU General\n"
			 "Public License as published by the Free Software Foundation.\n",
			 VERSION);
}

void 
usage(QApplication& a)
{
	qWarning("Usage: %s [options] <filename1> [<filename2> ...]", a.argv()[0]);
	qWarning("   --help               - print this message\n"
			 "   --version            - print the version and copyright info\n"
			 "   -v                   - same as '--version'\n"
			 "   --debug N            - print debug output, N must be between\n"
			 "                          0 and 4, the higher N the more output\n"	         "                          is printed\n"
			 "   --updatedb           - update the Kotrus database with the\n"
			 "                          new resource usage information\n");
}

int main(int argc, char *argv[])
{
	QApplication a(argc, argv, false);

	int debugLevel = 0;
	bool updateKotrusDB = FALSE;

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
				showCopyright = showHelp = terminateProgram = 1;
			debugLevel = QString(a.argv()[++i]).toInt();
		}
		else if (strcmp(a.argv()[i], "--version") == 0 ||
				 strcmp(a.argv()[i], "-v") == 0)
			showCopyright = TRUE;
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

	Project p;
	p.setDebugLevel(debugLevel);

	bool parseErrors = FALSE;

	for ( ; i < argc; i++)
	{
		ProjectFile* pf = new ProjectFile(&p);
		pf->setDebugLevel(debugLevel);
		if (!pf->open(a.argv()[i]))
			return (-1);
		parseErrors = !pf->parse();
	}

	p.readKotrus();

	if (!p.pass2())
		return (1);

	if (updateKotrusDB)
		if (parseErrors)
			qWarning("Due to parse errors the Kotrus DB will NOT be "
					 "updated.");
		else
			p.updateKotrus();

	p.generateReports();

	return (parseErrors ? -1 : 0);
}
