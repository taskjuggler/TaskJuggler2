/*
 * Project.h - TaskJuggler
 *
 * Copyright (c) 2001 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 */

#ifndef _Project_h_
#define _Project_h_

#include "time.h"

#include <qlist.h>

#include "Task.h"
#include "ResourceList.h"
#include "VacationList.h"

class Project
{
public:
	Project()
	{
		start = 0;
		end = (time_t) ~((time_t) 0);
		minEffort = 0.0;
		maxEffort = 1.0;
		rate = 0.0;
	}
	~Project() { }

	bool addTask(Task* t);
	void printText();
	bool pass2();

	void setStart(time_t s) { start = s; }
	time_t getStart() const { return start; }

	void setEnd(time_t e) { end = e; }
	time_t getEnd() const { return end; }

	void setId(const QString& i) { id = i; }
	const QString& getId() const { return id; }

	void setName(const QString& n) { name = n; }
	const QString& getName() const { return name; }

	void setManager(const QString& m) { manager = m; }
	const QString& getManager() const { return manager; }

	void addVacation(const QString& n, time_t s, time_t e)
	{
		vacationList.add(n, s, e);
	};
	bool isVacationDay(time_t d) { return vacationList.isVacationDay(d); }

	void addResource(Resource* r)
	{
		resourceList.add(r);
	}
	Resource* getResource(const QString& id)
	{
		return resourceList.getResource(id);
	}

	void setMinEffort(double m) { minEffort = m; }
	double getMinEffort() const { return minEffort; }

	void setMaxEffort(double m) { maxEffort = m; }
	double getMaxEffort() const { return maxEffort; }

	void setRate(double r) { rate = r; }
	double getRate() const { return rate; }

private:
	bool checkSchedule();

	int unscheduledTasks();
	time_t start;
	time_t end;

	QString id;
	QString name;
	QString manager;

	// Default values for Resource variables
	double minEffort;
	double maxEffort;
	double rate;

	QList<Task> taskList;
	ResourceList resourceList;
	VacationList vacationList;
} ;

#endif
