/*
 * Project.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _Project_h_
#define _Project_h_

#include <time.h>
#include <stdio.h>

#include <qlist.h>

#include "ShiftList.h"
#include "Task.h"
#include "ResourceList.h"
#include "VacationList.h"
#include "Account.h"
#include "HTMLTaskReport.h"
#include "HTMLResourceReport.h"
#include "HTMLAccountReport.h"
#include "ExportReport.h"
#include "ReportXML.h"
#ifdef HAVE_ICAL
#ifdef HAVE_KDE
#include "ReportICal.h"
#endif
#endif
class Kotrus;

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

	void setWorkingHours(int day, QPtrList<Interval>* l)
	{
		if (day < 0 || day > 6)
			qFatal("day out of range");
		delete workingHours[day];
		workingHours[day] = l;
	}
	QPtrList<Interval>* getWorkingHours(int day)
	{
		if (day < 0 || day > 6)
			qFatal("day out of range");
		return workingHours[day];
	}
	bool isWorkingDay(time_t d)
	{
		/* If there is a working interval defined for this weekday and the
		 * day is not registered as a vacation day then it is a workday. */
		return !(workingHours[dayOfWeek(d)]->isEmpty() || isVacation(d));
	}
	
	Interval* getVacationListFirst()
	{
		return vacationList.first(); 
	}
	Interval* getVacationListNext()
	{
		return vacationList.next();
	}

	Task* getTask(const QString& id)
	{
		return taskList.getTask(id);
	}
	uint taskCount() const { return taskList.count(); }

	void addResource(Resource* r)
	{
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
	uint accountCount() const { return accountList.count(); }

	void addShift(Shift* s)
	{
		shiftList.inSort(s);
	}
	Shift* getShift(const QString& id)
	{
		return shiftList.getShift(id);
	}
	uint shiftCount() const { return shiftList.count(); }

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

	double getWeeklyWorkingDays() const
   	{
	   	return yearlyWorkingDays / 52.14285714; 
	}

	double getMonthlyWorkingDays() const 
	{ 
		return yearlyWorkingDays / 12; 
	}

	void setYearlyWorkingDays(double d) { yearlyWorkingDays = d; }
	double getYearlyWorkingDays() const { return yearlyWorkingDays; }

	void setScheduleGranularity(ulong s) { scheduleGranularity = s; }
	ulong getScheduleGranularity() const { return scheduleGranularity; }

	void addXMLReport(ReportXML *r ) { xmlreport = r; }

   bool loadFromXML( const QString& file );
   void parseDomElem( QDomElement& parentElem );

#ifdef HAVE_ICAL
#ifdef HAVE_KDE
	void addICalReport( ReportICal *ic ) { icalReport = ic; }
#endif
#endif

	void addHTMLTaskReport(HTMLTaskReport* h) { htmlTaskReports.append(h); }

	void addHTMLResourceReport(HTMLResourceReport* r)
	{
		htmlResourceReports.append(r);
	}

	void addHTMLAccountReport(HTMLAccountReport* a)
	{
		htmlAccountReports.append(a);
	}

	void addExportReport(ExportReport* e)
	{
		exportReports.append(e);
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

	void setKotrus(Kotrus* k);
	Kotrus* getKotrus() const { return kotrus; }

	bool readKotrus();
	bool updateKotrus();

	TaskList getTaskList() { return taskList; }
	ResourceList getResourceList() { return resourceList; }
	AccountList getAccountList() { return accountList; }

	void generateReports();
	bool needsActualDataForReports();
	void removeActiveTask(Task* t);
	void addActiveTask(Task* t);

	static void setDebugLevel(int l)
   	{
	   	debugLevel = l;
		Task::setDebugLevel(l);
		Resource::setDebugLevel(l);
	}

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

	/* The average number of working hours per day. This factor is used
	 * when converting hours in working days. It should match the workingHours
	 * closely. */
	double dailyWorkingHours;

	/* The average number of working days per year. This factor is used when
	 * converting working days into years. It should match the defined working
	 * hours and vacation days. */
	double yearlyWorkingDays;

	/* The list of standard working or opening hours. These values will be
	 * inherited by the resources. */
	QList<Interval>* workingHours[7];
	
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

	ShiftList shiftList;
	TaskList taskList;
	ResourceList resourceList;
	VacationList vacationList;
	AccountList accountList;

	Kotrus* kotrus;

	ReportXML* xmlreport;
#ifdef HAVE_ICAL
#ifdef HAVE_KDE
	ReportICal *icalReport;
#endif
#endif
   
	QList<HTMLTaskReport> htmlTaskReports;
	QList<HTMLResourceReport> htmlResourceReports;
	QList<HTMLAccountReport> htmlAccountReports;
	QList<ExportReport> exportReports;

	static int debugLevel;

	/**
	 * The task lists for active ASAP and ALAP tasks are only used
	 * during the scheduling process.
	 */
	TaskList activeAsap;
	TaskList activeAlap;
} ;

#endif
