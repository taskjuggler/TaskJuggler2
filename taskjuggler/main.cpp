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

#include <qapplication.h>

#include "Project.h"
#include "ProjectFile.h"

int main(int argc, char *argv[])
{
	
	QApplication a(argc, argv, false);

	Project p;
	ProjectFile* pf = new ProjectFile(&p);
	if (!pf->open("test.task"))
		return (-1);
	pf->parse();
	p.pass2();
	p.reportHTMLTaskList();
	p.reportHTMLResourceList();

	return (0);
}
