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


/* ugly: */
Kotrus *kotrus;

void 
usage(QApplication& a)
{
	qWarning("TaskJuggler v%s - A Project Management Software\n"
			 "Copyright (c) 2001, 2002 Chris Schlaeger <cs@suse.de> and\n"
			 "                         Klaas Freitag <freitag@suse.de>\n"
			 "This program is free software; you can redistribute it and/or\n"
			 "modify it under the terms of version 2 of the GNU General\n"
			 "Public License as published by the Free Software Foundation.\n",
			 VERSION);
	qWarning("Usage: %s [options] <filename1> [<filename2> ...]", a.argv()[0]);
	exit(1);
}

int main(int argc, char *argv[])
{
	QApplication a(argc, argv, false);

	int debugLevel = 0;
	
	int i;
	for (i = 1 ; i < a.argc() && a.argv()[i][0] == '-'; i++)
		if (strcmp(a.argv()[i], "--debug") == 0)
		{
			if (i + 1 >= a.argc())
				usage(a);
			debugLevel = QString(a.argv()[++i]).toInt();
		}
	
	if (a.argc() - i < 1)
		usage(a);
	
	Project p;
	p.setDebugLevel(debugLevel);

	kotrus = new Kotrus();

	bool parseErrors = FALSE;

	for ( ; i < argc; i++)
	{
		ProjectFile* pf = new ProjectFile(&p);
		if (!pf->open(a.argv()[i]))
			return (-1);
		parseErrors = !pf->parse();
	}
	if (!p.pass2())
		return (1);
	p.generateReports();

	return (parseErrors ? -1 : 0);
}
