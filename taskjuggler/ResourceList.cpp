/*
 * ResourceList.cpp - TaskJuggler
 *
 * Copyright (c) 2001 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <stdio.h>

#include "ResourceList.h"
#include "Task.h"
#include "Project.h"

double
Resource::isAvailable(time_t date)
{
	double bookedEffort = 0.0;
	for (Booking* b = jobs.first(); b != 0; b = jobs.next())
		if (date == b->getDate())
			bookedEffort += b->getEffort();
	return (maxEffort - bookedEffort);
}

bool
Resource::book(Booking* b)
{
	jobs.append(b);
	return TRUE;
}

double
Resource::getLoadOnDay(time_t date)
{
	double load = 0.0;

	for (Booking* b = jobs.first(); b != 0; b = jobs.next())
		if (date == b->getDate())
			load += b->getEffort();
	return load;
}

double
Resource::getLoadOnDay(time_t date, Task* task)
{
	double load = 0.0;

	for (Booking* b = jobs.first(); b != 0; b = jobs.next())
		if (date == b->getDate() && task == b->getTask())
			load += b->getEffort();
	return load;
}

bool
Resource::isBusyWith(Task* task)
{
	for (Booking* b = jobs.first(); b != 0; b = jobs.next())
		if (task == b->getTask())
			return TRUE;
	return FALSE;
}

int
ResourceList::compareItems(QCollection::Item i1, QCollection::Item i2)
{
	Resource* r1 = static_cast<Resource*>(i1);
	Resource* r2 = static_cast<Resource*>(i2);

	return r1->getId().compare(r2->getId());
}

Resource*
ResourceList::getResource(const QString& id)
{
	Resource key(id, "");
	return at(find(&key));
}

void
Resource::printText()
{
	printf("ID: %s\n", id.latin1());
	printf("Name: %s\n", name.latin1());
	printf("MinEffort: %3.2f  MaxEffort: %3.2f  Rate: %7.2f\n",
		   minEffort, maxEffort, rate);
	for (Booking* j = jobs.first(); j != 0; j = jobs.next())
		printf("%s %5.2f %s\n", time2ISO(j->getDate()).latin1(),
			   j->getEffort(),
			   j->getTask()->getId().latin1());
}

void
ResourceList::printText()
{
	for (Resource* r = first(); r != 0; r = next())
		r->printText();
}


