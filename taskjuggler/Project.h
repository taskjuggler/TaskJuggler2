/*
 * Project.h - TaskJuggler
 *
 * Copyright (c) 2001 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _Project_h_
#define _Project_h_

#include <time.h>
#include <stdio.h>

#include <qlist.h>

#include "Task.h"
#include "ResourceList.h"
#include "VacationList.h"
#include "Account.h"

class Project
{
public:
	Project();
	~Project() { }

	bool addTask(Task* t);
	bool pass2();

	void printText();

	bool reportHTMLTaskList();
	bool reportHTMLHeader(FILE* f);
	bool reportHTMLFooter(FILE* f);

	bool reportHTMLResourceList();

	void setId(const QString& i) { id = i; }
	const QString& getId() const { return id; }

	void setName(const QString& n) { name = n; }
	const QString& getName() const { return name; }

	void setVersion(const QString& v) { version = v; }
	const QString& getVersion() const { return version; }

	void setPriority(int p) { priority = p; }
	int getPriority() const { return priority; }

	void setStart(time_t s) { start = s; }
	time_t getStart() const { return start; }

	void setEnd(time_t e) { end = e; }
	time_t getEnd() const { return end; }

	void addVacation(const QString& n, time_t s, time_t e)
	{
		vacationList.add(n, s, e);
	}
	bool isVacation(time_t d) { return vacationList.isVacation(d); }

	void addResource(Resource* r)
	{
		resourceList.append(r);
	}
	Resource* getResource(const QString& id)
	{
		return resourceList.getResource(id);
	}

	void addAccount(Account* a)
	{
		accountList.inSort(a);
	}
	Account* getAccount(const QString& id)
	{
		return accountList.getAccount(id);
	}

	void setMinEffort(double m) { minEffort = m; }
	double getMinEffort() const { return minEffort; }

	void setMaxEffort(double m) { maxEffort = m; }
	double getMaxEffort() const { return maxEffort; }

	void setRate(double r) { rate = r; }
	double getRate() const { return rate; }

	void setDailyWorkingHours(double h) { dailyWorkingHours = h; }
	double getDailyWorkingHours() const { return dailyWorkingHours; }

	void setScheduleGranularity(ulong s) { scheduleGranularity = s; }
	ulong getScheduleGranularity() const { return scheduleGranularity; }

	void setHtmlTaskReport(const QString& f) { htmlTaskReport = f; }
	void addHtmlTaskReportColumn(const QString& c)
	{
		htmlTaskReportColumns.append(c);
	}
	void setHtmlTaskReportStart(time_t s) { htmlTaskReportStart = s; }
	void setHtmlTaskReportEnd(time_t e) { htmlTaskReportEnd = e; }

	void setHtmlResourceReport(const QString& f) { htmlResourceReport = f; }
	void setHtmlResourceReportStart(time_t t) { htmlResourceReportStart = t; }
	void setHtmlResourceReportEnd(time_t t) { htmlResourceReportEnd = t; }

	void addAllowedFlag(QString flag)
	{
		if (!isAllowedFlag(flag))
			allowedFlags.append(flag);
	}
	bool isAllowedFlag(const QString& flag)
	{
		return allowedFlags.contains(flag) > 0;
	}

	double convertToDailyLoad(long secs)
	{
		return ((double) secs / (dailyWorkingHours * ONEHOUR));
	}
	
private:
	bool checkSchedule();
	void htmlDayHeader(FILE* f, time_t s, time_t e);
	void htmlMonthHeader(FILE* f, time_t s, time_t e);
	QString htmlFilter(const QString& s);

	int unscheduledTasks();
	time_t start;
	time_t end;

	// A unique (with respect to the resource database) project id
	QString id;
	// The name of the Project
	QString name;
	// The revision of the project description.
	QString version;

	/* The default priority that will be inherited by all tasks. Sub tasks
	 * will inherit the priority of its parent task. */
	int priority;

	// Default values for Resource variables
	double minEffort;
	double maxEffort;
	double rate;

	// Number of working hours of a generic working day.
	double dailyWorkingHours;

	/* The granularity of the scheduler in seconds. No intervals shorter
	 * than this time will be scheduled. */
	ulong scheduleGranularity;

	/* To be able to detect flag conflicts between multiple parts of a project
	 * all flags must be registered before they can be used. This variable
	 * contains the list of all registered flags. Some flags like 'hidden'
	 * or 'closed' are pre-registered and will be set automatically. */
	QStringList allowedFlags;

	TaskList taskList;
	ResourceList resourceList;
	VacationList vacationList;
	AccountList accountList;

	QString htmlTaskReport;
	QStringList htmlTaskReportColumns;
	time_t htmlTaskReportStart;
	time_t htmlTaskReportEnd;

	QString htmlResourceReport;
    time_t htmlResourceReportStart;
	time_t htmlResourceReportEnd;
} ;

#endif
