/*
 * main.cpp - TaskJuggler
 *
 * Copyright (c) 2001 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 */

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
	qWarning("Usage: %s <filename>", a.argv()[0]);
	exit(1);
}

int main(int argc, char *argv[])
{
	QApplication a(argc, argv, false);

	if (argc != 2)
		usage(a);

	Project p;
	kotrus = new Kotrus();
	
	ProjectFile* pf = new ProjectFile(&p);
	if (!pf->open(a.argv()[1]))
		return (-1);
	pf->parse();
	p.pass2();

	p.generateReports();

	return (0);
}
