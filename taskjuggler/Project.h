/*
 * Project.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
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
#include "TaskList.h"
#include "ResourceList.h"
#include "VacationList.h"
#include "AccountList.h"
#include "HTMLTaskReport.h"
#include "HTMLResourceReport.h"
#include "HTMLAccountReport.h"
#include "HTMLWeeklyCalendar.h"
#include "ExportReport.h"
#include "ReportXML.h"
#ifdef HAVE_ICAL
#ifdef HAVE_KDE
#include "ReportICal.h"
#endif
#endif
class Kotrus;

/**
 * The Project class is the root of the data tree of the application.
 * Applications can use multiple Project objects. This is e. g. needed when
 * the application needs to preserve the state prior to a scheduler run. The
 * scheduler calculates the data in place, adding the calculated values to the
 * existing data structures. Since the original data cannot be identified
 * anymore the applications needs to make a copy of the project before calling
 * the scheduler. For that purpose Project and all related sub-classes provide
 * a copy constructor.
 * been tested.
 *
 * @short The root class of all project related infromation.
 * @author Chris Schlaeger <cs@suse.de>
 */
class Project
{
public:
	Project();
	~Project();

	/**
	 * Projects have at least one ID, but can have multiple IDs. This usually
	 * happens when projects are composed of serveral sub-projects. Each sub
	 * projects brings its own unique ID. Each ID must be registered with the
	 * project by calling addId(). The most recently added ID is also the
	 * current ID. All subsequently added tasks are associtated with this
	 * project ID. So, you have to add at least one ID before you add any
	 * tasks.
	 */
	bool addId(const QString& i);
	/**
	 * Returns the current project ID. If the project ID list is empty
	 * QString::null is returned.
	 */
	QString getCurrentId() const
	{
		return projectIDs.isEmpty() ? QString::null : projectIDs.last();
	}
	/**
	 * Returns a list of all registered project IDs.
	 */
	QStringList getProjectIdList() const { return projectIDs; }
	/**
	 * Returns TRUE if the passed ID is a registered project ID.
	 */
	bool isValidId(const QString& i) const
	{
		return projectIDs.findIndex(i) != -1;
	}
	/**
	 * Returns a string ID of the index of the passed ID in the project ID.
	 * The first ID in the list is returned as "A", the second as "B". The
	 * 27th is "AA" and so on.
	 */
	QString getIdIndex(const QString& i) const;
	
	/**
	 * Returns the number of supported scenarios.
	 */
	int getMaxScenarios() { return scenarioNames.count(); }
	/**
	 * Returns the name of a scenario.
	 * @param sc Specifies the scenario.
	 */
	const QString& getScenarioName(int sc);

	/**
	 * Set the name of the project. The project name is mainly used for the
	 * reports.
	 */
	void setName(const QString& n) { name = n; }
	/**
	 * Returns the name of the project.
	 */
	const QString& getName() const { return name; }

	/**
	 * Set the version number of the project. This version is maily used for
	 * reports.
	 */
	void setVersion(const QString& v) { version = v; }
	/**
	 * Returns the version number of the project.
	 */
	const QString& getVersion() const { return version; }

	/**
	 * Set the copyright information. This is a default text used for all
	 * reports.
	 */
	void setCopyright(const QString& c) { copyright = c; }
	/**
	 * Returns the copyright information of the project.
	 */
	const QString& getCopyright() const { return copyright; }
	
	/**
	 * Set the default priority for all top-level tasks. Normally this value
	 * is 500.
	 */
	void setPriority(int p) { priority = p; }
	/**
	 * Returns the default priority for all top-level tasks.
	 */
	int getPriority() const { return priority; }

	/**
	 * Set the start time of the project.
	 */
	void setStart(time_t s) { start = s; }
	/**
	 * Get the start time of the project.
	 */
	time_t getStart() const { return start; }

	/**
	 * Set the end time of the project. The specified second is still
	 * considered as within the project time frame.
	 */
	void setEnd(time_t e) { end = e; }
	/**
	 * Get the end time of the project.
	 */
	time_t getEnd() const { return end; }

	/**
	 * Set the date that TaskJuggler uses as current date for all
	 * computations. This mainly affects status reporting and the computation
	 * of the completion degree of tasks. */
	void setNow(time_t n) { now = n; }
	/**
	 * Get the date that TaskJuggler uses as current date.
	 */
	time_t getNow() const { return now; }

	/**
	 * Specifies whether TaskJuggler uses Sunday or Monday as first day of the
	 * week. Besides the calendar views this mainly affects the numbering of
	 * weeks. ISO 8601:1988 requires the week to start on Monday as most
	 * European countries use, but other Countries like the US use Sunday as
	 * first day of the week.
	 */
	void setWeekStartsMonday(bool wsm) { weekStartsMonday = wsm; }
	/**
	 * Get the setting for the first day of the week.
	 * @return TRUE of weeks should start on Monday.
	 */
	bool getWeekStartsMonday() const { return weekStartsMonday; }

	/**
	 * Set the working hours of the specified weekday.
	 * @param day The day of the week. Independently of the weekStartsMonday
	 * setting TaskJuggler uses 0 for Sunday, 1 for Monday and so on.
	 * @param l The list of working intervals. The interval use seconds since
	 * midnight. As with all TaskJuggler intervals, the specified end value is
	 * not part of the interval. The interval ends one seconds earlier.
	 */
	void setWorkingHours(int day, QPtrList<Interval>* l)
	{
		if (day < 0 || day > 6)
			qFatal("day out of range");
		delete workingHours[day];
		workingHours[day] = l;
	}
	/**
	 * Returns the list of working intervals for the specified weekday.
	 * @param day Day of the week. 0 for Sunday, 1 for Monday and so on.
	 */
	QPtrList<Interval>* getWorkingHours(int day)
	{
		if (day < 0 || day > 6)
			qFatal("day out of range");
		return workingHours[day];
	}
	/**
	 * If there is a working interval defined for this weekday and the
	 * day is not registered as a vacation day then it is a workday. 
	 */
	bool isWorkingDay(time_t d)
	{
		return !(workingHours[dayOfWeek(d, FALSE)]->isEmpty() || isVacation(d));
	}
	/**
	 * Returns the number of working days that overlap with the specified
	 * interval.
	 */
	int calcWorkingDays(const Interval& iv);

	/**
	 * Add a vacation interval to the vacation list. These global vacations
	 * are meant for events like Christmas, Eastern or corporate hollidays.
	 * A day that overlaps any of the intervals in the list is considered to
	 * not be a working day. Vacation intervals may not overlap.
	 * @param n Name of the vacation.
	 * @param i The time interval the vacation lasts.
	 */	 
	void addVacation(const QString& n, const Interval& i)
	{
		vacationList.add(n, i);
	}
	/**
	 * Returns TRUE if the passed moment falls within any of the vacation
	 * intervals.
	 */
	bool isVacation(time_t d) { return vacationList.isVacation(d); }
	/**
	 * Returns the first interval in the vacation list. It also sets the
	 * current list item to the first item.
	 */
	Interval* getVacationListFirst()
	{
		return vacationList.first(); 
	}
	/**
	 * Returns the interval after the current one and sets the current to the
	 * next in the list.
	 */
	Interval* getVacationListNext()
	{
		return vacationList.next();
	}

	void addTask(Task* t)
	{
		taskList.append(t);
	}
	Task* getTask(const QString& id)
	{
		return taskList.getTask(id);
	}
	uint taskCount() const { return taskList.count(); }

	void addResource(Resource* r);
	
	Resource* getResource(const QString& id)
	{
		return resourceList.getResource(id);
	}
	uint resourceCount() const { return resourceList.count(); }

	void addAccount(Account* a)
	{
		accountList.append(a);
	}
	Account* getAccount(const QString& id)
	{
		return accountList.getAccount(id);
	}
	uint accountCount() const { return accountList.count(); }

	void addShift(Shift* s);
	
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

	void addHTMLWeeklyCalendar(HTMLWeeklyCalendar* r)
	{
		htmlWeeklyCalendars.append(r);
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

	double convertToDailyLoad(long secs) const
	{
		return ((double) secs / (dailyWorkingHours * ONEHOUR));
	}

	void setKotrus(Kotrus* k);
	Kotrus* getKotrus() const { return kotrus; }

	bool readKotrus();
	bool updateKotrus();

	void setShortTimeFormat(const QString& tf) { shortTimeFormat = tf; }
	const QString& getShortTimeFormat() const { return shortTimeFormat; }

	void setTimeFormat(const QString& tf) { timeFormat = tf; }
	const QString& getTimeFormat() const { return timeFormat; }

	TaskList getTaskList() { return taskList; }
	ResourceList getResourceList() { return resourceList; }
	AccountList getAccountList() { return accountList; }

	/**
	 * Generate cross references between all data structures and run a
	 * consistency check. This function must be called after the project data
	 * tree has been contructed. 
	 * @return Only if all tests were successful TRUE is returned.
	 */
	bool pass2();

	/**
	 * Schedule all tasks for all scenarios. @return In case any errors were
	 * detected FALSE is returned.
	 */
	bool scheduleAllScenarios();
	void generateReports();
	bool needsActualDataForReports();

private:
	void overlayScenario(int sc);
	void prepareScenario(int sc);
	void finishScenario(int sc);
	
	bool schedule(const QString& scenario);

	bool checkSchedule(const QString& scenario);

	/// The start date of the project
	time_t start;
	/// The end date of the project
	time_t end;
	/// The current date used in status calculations and reports.
	time_t now;

	/// True if week based calculations use Monday as first day of week.
	bool weekStartsMonday;

	/// The name of the Project
	QString name;
	/// The revision of the project description.
	QString version;
	/// Some legal words to please the boss.
	QString copyright;

	/**
	 * A format string in strftime(3) format that specifies the default time
	 * format for all time values TaskJuggler generates.
	 */
	QString timeFormat;
	/**
	 * A format string in strftime(3) format that specifies the time format
	 * for all daytime values (e. g. HH:MM).
	 */
	QString shortTimeFormat;

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
	QPtrList<Interval>* workingHours[7];
	
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

	QStringList scenarioNames;
	bool hasExtraValues;	// TODO: Fix this for multiple scenarios

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
   
	QPtrList<HTMLTaskReport> htmlTaskReports;
	QPtrList<HTMLResourceReport> htmlResourceReports;
	QPtrList<HTMLAccountReport> htmlAccountReports;
	QPtrList<HTMLWeeklyCalendar> htmlWeeklyCalendars;
	QPtrList<ExportReport> exportReports;
} ;

#endif
