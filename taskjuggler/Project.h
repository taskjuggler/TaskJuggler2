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
#include "HTMLTaskReport.h"
#include "HTMLResourceReport.h"
#include "HTMLAccountReport.h"
#include "ReportXML.h"

class Project
{
public:
	Project();
	~Project() { }

	bool addTask(Task* t);
	bool pass2();

	void preparePlan();
	void finishPlan();
	void prepareActual();
	void finishActual();
	bool schedule();

	void setName(const QString& n) { name = n; }
	const QString& getName() const { return name; }

	void setVersion(const QString& v) { version = v; }
	const QString& getVersion() const { return version; }

	void setCopyright(const QString& c) { copyright = c; }
	const QString& getCopyright() const { return copyright; }

	void setPriority(int p) { priority = p; }
	int getPriority() const { return priority; }

	void setStart(time_t s) { start = s; }
	time_t getStart() const { return start; }

	void setEnd(time_t e) { end = e; }
	time_t getEnd() const { return end; }

	void setNow(time_t n) { now = n; }
	time_t getNow() const { return now; }
	
	bool addId(const QString& i);
	QString getCurrentId() const
	{
		return projectIDs.isEmpty() ? QString() : projectIDs.last();
	}
	QStringList getProjectIdList() const { return projectIDs; }
	bool isValidId(const QString& i) const
	{
		return projectIDs.findIndex(i) != -1;
	}
	QString getIdIndex(const QString& i) const;

	void addVacation(const QString& n, time_t s, time_t e)
	{
		vacationList.add(n, s, e);
	}
	bool isVacation(time_t d) { return vacationList.isVacation(d); }

	void addResource(Resource* r)
	{
	   r->dbLoadBookings( r->getKotrusId(), 0 );
		resourceList.append(r);
		
	}
	Resource* getResource(const QString& id)
	{
		return resourceList.getResource(id);
	}
	uint resourceCount() const { return resourceList.count(); }

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

	void setCurrency(const QString& s) { currency = s; }
	const QString& getCurrency() const { return currency; }

	void setCurrencyDigits(int d) { currencyDigits = d; }
	int getCurrencyDigits() const { return currencyDigits; }

	void setDailyWorkingHours(double h) { dailyWorkingHours = h; }
	double getDailyWorkingHours() const { return dailyWorkingHours; }

	void setScheduleGranularity(ulong s) { scheduleGranularity = s; }
	ulong getScheduleGranularity() const { return scheduleGranularity; }

	void addXMLReport(ReportXML *r ) { xmlreport = r; }
   
	void addHTMLTaskReport(HTMLTaskReport* h) { htmlTaskReports.append(h); }

	void addHTMLResourceReport(HTMLResourceReport* r)
	{
		htmlResourceReports.append(r);
	}

	void addHTMLAccountReport(HTMLAccountReport* a)
	{
		htmlAccountReports.append(a);
	}

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

	TaskList getTaskList() { return taskList; }
	ResourceList getResourceList() { return resourceList; }
	AccountList getAccountList() { return accountList; }

	void generateReports();
	bool needsActualDataForReports();
	void removeActiveTask(Task* t);
	void addActiveTask(Task* t);
   
private:
	bool checkSchedule();
	void updateActiveTaskList(TaskList& sortedTasks);

	/// The start date of the project
	time_t start;
	/// The end date of the project
	time_t end;
	/// The current date used in reports.
	time_t now;

	/// The name of the Project
	QString name;
	/// The revision of the project description.
	QString version;
	/// Some legal words to please the boss.
	QString copyright;

	/**
	 * The default priority that will be inherited by all tasks. Sub tasks
	 * will inherit the priority of its parent task. */
	int priority;

	/// Default values for Resource variables
	double minEffort;
	double maxEffort;
	double rate;

	/// The currency of used for all money values.
	QString currency;
	/// The number of fraction digits of all money values.
	int currencyDigits;

	/// Number of working hours of a generic working day.
	double dailyWorkingHours;

	/**
	 * The granularity of the scheduler in seconds. No intervals
	 * shorter than this time will be scheduled. */
	ulong scheduleGranularity;

	/**
	 * To avoid difficult to find typos in flag names all flags must
	 * be registered before they can be used. This variable contains
	 * the list of all registered flags. It is legal to declare a flag
	 * twice, so we can merge projects to a larger project. */
	QStringList allowedFlags;

	/**
	 * Each project has a unique ID but can have multiple other IDs as
     * well. This happens usually when small projects are merged to a
     * create a big project. Each task can be assigned to a different
     * project ID but all IDs must be declared before they can be
     * used. */
	QStringList projectIDs;

	TaskList taskList;
	ResourceList resourceList;
	VacationList vacationList;
	AccountList accountList;

	TaskList activeAsap;
	TaskList activeAlap;

	ReportXML *xmlreport;
   
	QList<HTMLTaskReport> htmlTaskReports;
	QList<HTMLResourceReport> htmlResourceReports;
	QList<HTMLAccountReport> htmlAccountReports;
} ;

#endif
