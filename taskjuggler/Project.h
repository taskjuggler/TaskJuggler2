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

#include <qptrlist.h>
#include <qstringlist.h>
#include <qdict.h>

#include "VacationList.h"
#include "ScenarioList.h"
#include "TaskList.h"
#include "ShiftList.h"
#include "ResourceList.h"
#include "AccountList.h"
#include "RealFormat.h"

class QStringList;
class QDomElement;

class ReportXML;
class Report;
class Kotrus;
class CustomAttributeDefinition;
class VacationInterval;

/**
 * The Project class is the root of the data tree of the application.
 * Applications can use multiple Project objects. This is e. g. needed when
 * the application needs to preserve the state prior to a scheduler run. The
 * scheduler calculates the data in place, adding the calculated values to the
 * existing data structures. Since the original data cannot be identified
 * anymore the applications needs to make a copy of the project before calling
 * the scheduler. For that purpose Project and all related sub-classes provide
 * a copy constructor.
 *
 * @short The root class of all project related information.
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
    bool addId(const QString& i, bool changeCurrentId = TRUE);

    /**
     * Returns the first (default) ID of the project.
     */
    QString getId() const
    { 
        return projectIDs.isEmpty() ? QString::null : projectIDs.first();
    }
    /**
     * Returns the current project ID. If the project ID list is empty
     * QString::null is returned.
     */
    QString getCurrentId() const
    {
        return currentId;
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
    int getMaxScenarios() const { return scenarioList.count(); }
    /**
     * Returns a pointer to the scenario.
     * @param sc Index of the scenario in the project scenario list.
     */
    Scenario* getScenario(int sc) const;
    /**
     * Returns a pointer to the scenario with index sc.
     * @param sc Specifies the scenario.
     */
    const QString& getScenarioId(int sc) const;
    /**
     * Returns the name of a scenario.
     * @param sc Specifies the scenario.
     */
    const QString& getScenarioName(int sc) const;
    /**
     * Returns the index of the scenario. The index of the first item in the
     * list is 1, not 0! If the scenario is unknown -1 is returned.
     * @param id the ID of the scenario.
     */
    int getScenarioIndex(const QString& id) const;

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
     * Returns a constant list of working intervals for all week days.
     */
    const QPtrList<const Interval>* const * getWorkingHours() const
    {
        return (const QPtrList<const Interval>* const*) workingHours;
    }
    /**
     * Returns the list of working intervals for the specified weekday.
     * @param day Day of the week. 0 for Sunday, 1 for Monday and so on.
     */
    QPtrList<Interval>* getWorkingHours(int day) const
    {
        if (day < 0 || day > 6)
            qFatal("day out of range");
        return workingHours[day];
    }
    /**
     * Returns an interator for the list of working intervals for the
     * specified weekday.
     * @param day Day of the week. 0 for Sunday, 1 for Monday and so on.
     */
    QPtrListIterator<Interval> getWorkingHoursIterator(int day) const
    {
        return QPtrListIterator<Interval>(*workingHours[day]);
    }
    /**
     * If there is a working interval defined for this weekday and the
     * day is not registered as a vacation day then it is a workday. 
     */
    bool isWorkingDay(time_t d) const;
    
    /**
     * If the interval overlaps with a vacation day or the interval is outside
     * of the defined working hours, FALSE is returned. Otherwise TRUE.
     */
    bool isWorkingTime(const Interval& iv) const;

    /**
     * Returns the number of working days that overlap with the specified
     * interval.
     */
    int calcWorkingDays(const Interval& iv) const;

    /**
     * The daily working hours value is used to convert working hours into
     * working days. It should be an avarage value of the specified
     * workingHours for each week day. With this function you can set the
     * value for the project.
     */
    void setDailyWorkingHours(double h) { dailyWorkingHours = h; }
    /**
     * Returns the specified daily working hours.
     */
    double getDailyWorkingHours() const { return dailyWorkingHours; }

    /**
     * The weekly working days value is used to convert working days into
     * working weeks. It should match the number of working days specified
     * with the workingHours. The value is derived from the yearlyWorkingDays
     * setting. This function returns the value.
     */
    double getWeeklyWorkingDays() const
    {
        return yearlyWorkingDays / 52.1429; 
    }

    /**
     * The monthly working days value is used to convert working days into
     * working month. It should reflect the workingHours settings and the
     * vacation settings. The value is derived from the yearlyWorkingDays
     * setting. This function returns the value.
     */
    double getMonthlyWorkingDays() const 
    { 
        return yearlyWorkingDays / 12; 
    }

    /**
     * The yearly working days value is used to convert working days into
     * working years. The value should reflect the workingHours settings and
     * the vacation settings. This function sets the value which also affects
     * the monthly working days and the weekly working days.
     */
    void setYearlyWorkingDays(double d) { yearlyWorkingDays = d; }
    /**
     * Returns the specified number of working days per year.
     */
    double getYearlyWorkingDays() const { return yearlyWorkingDays; }

    void setScheduleGranularity(ulong s) { scheduleGranularity = s; }
    ulong getScheduleGranularity() const { return scheduleGranularity; }

    void setAllowRedefinitions(bool ar) { allowRedefinitions = ar; }
    bool getAllowRedefinitions() const { return allowRedefinitions; }

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
    void addVacation(VacationInterval* vi)
    {
        vacationList.add(vi);
    }
    /**
     * Returns TRUE if the passed moment falls within any of the vacation
     * intervals.
     */
    bool isVacation(time_t d) const { return vacationList.isVacation(d); }
    /**
     * Returns an iterator for the vacation list. 
     */
    VacationListIterator getVacationListIterator() const
    {
        return VacationListIterator(vacationList); 
    }

    /**
     * This function is for library internal use only. Creating a Scenario
     * object with the project as parameter will automatically add it to the
     * scenario list of the project.
     */
    void addScenario(Scenario* r);
    /**
     * This function is for library internal use only. Deleting a Scenario
     * will automatically delete it from the respective list of the
     * project.
     */
    void deleteScenario(Scenario* s);
    
    /**
     * This function is for library internal use only. Creating a Task object
     * with the project as parameter will automatically add it to the Task
     * list of the project.
     */
    void addTask(Task* t);
    /**
     * This function is for library internal use only. Deleting a Task
     * will automatically delete it from the respective list of the
     * project.
     */
    void deleteTask(Task* t);
    /**
     * Returns a pointer to the Task with the specified ID. The ID must be an
     * absolute ID of the form "foo.bar". If no Task with the ID exists 0 is
     * returned.
     */
    Task* getTask(const QString& id) const
    {
        return taskList.getTask(id);
    }
    /**
     * Returns the number of tasks of the project.
     */
    uint taskCount() const { return taskList.count(); }
    /**
     * Returns a copy of the Task list of the project.
     */
    TaskList getTaskList() const { return taskList; }
    /**
     * Returns an iterator that can be used to traverse the Task list. The
     * Task list is a flat list of all tasks.
     */
    TaskListIterator getTaskListIterator() const
    {
        return TaskListIterator(taskList);
    }
    /**
     * This function adds a new, user-defined attribute to the Task
     * attributes. The @param id must be unique within the Task attributes
     * namespace. @param cad is a pointer to the CustomAttributeDefinition
     * object.
     */
    bool addTaskAttribute(const QString& id, CustomAttributeDefinition* cad);
    /**
     * Returns a pointer to the custom attribute object identified by @param id.
     * If no attributes with the id exists, 0 is returned.
     */
    const CustomAttributeDefinition* getTaskAttribute(const QString& id) const;
    /**
     * Returns a read-only pointer to the dictionary that holds the pointers
     * to user-defined attributes of Tasks.
     */
    const QDict<const CustomAttributeDefinition>&
        getTaskAttributeDict() const 
    { 
        return (const QDict<const CustomAttributeDefinition>&) taskAttributes; 
    }

    /**
     * This function is for library internal use only. Creating a Resource
     * object with the project as parameter will automatically add it to the
     * resource list of the project.
     */
    void addResource(Resource* r);
    /**
     * This function is for library internal use only. Deleting a Resource
     * will automatically delete it from the respective list of the
     * project.
     */
    void deleteResource(Resource* r);
    /**
     * Returns a pointer to the Resource with the specified ID. The ID must
     * not be an absolute ID since the Resource list has a flat namespace. If
     * no Resource with the ID exists 0 is returned.
     */
    Resource* getResource(const QString& id) const
    {
        return resourceList.getResource(id);
    }
    /**
     * Returns the number of resources in the Resource list.
     */
    uint resourceCount() const { return resourceList.count(); }
    /**
     * Returns a copy of the Resource list.
     */
    ResourceList getResourceList() const { return resourceList; }
    /**
     * Returns an iterator that can be used to traverse the Resource list. The
     * Resource list is a flat list of all resources.
     */
    ResourceListIterator getResourceListIterator() const
    {
        return ResourceListIterator(resourceList);
    }
    /**
     * This function adds a new, user-defined attribute to the Resource 
     * attributes. The @param id must be unique within the Resource attributes
     * namespace. @param cad is a pointer to the CustomAttributeDefinition
     * object.
     */
    bool addResourceAttribute(const QString& name, 
                              CustomAttributeDefinition* cad);
    /**
     * Returns a pointer to the custom attribute object identified by @param id.
     * If no attributes with the id exists, 0 is returned.
     */
    const CustomAttributeDefinition* getResourceAttribute(const QString& id)
        const;
    /**
     * Returns a read-only pointer to the dictionary that holds the pointers
     * to user-defined attributes of Resources.
     */
    const QDict<const CustomAttributeDefinition>&
        getResourceAttributeDict() const 
    { 
        return (const QDict<const CustomAttributeDefinition>&) 
            resourceAttributes; 
    }

    /**
     * This function is for library internal use only. Creating an Account 
     * object with the project as parameter will automatically add it to the
     * Account list of the project.
     */
    void addAccount(Account* a);
    /**
     * This function is for library internal use only. Deleting a Account
     * will automatically delete it from the respective list of the
     * project.
     */
    void deleteAccount(Account* a);
    /**
     * Returns a pointer to the Account with the specified ID. The ID may
     * not be an absolute ID since the account list has a flat namespace. If
     * no Account with the ID exists 0 is returned.
     */
    Account* getAccount(const QString& id) const
    {
        return accountList.getAccount(id);
    }
    /**
     * Returns the number of accounts in the Account list.
     */
    uint accountCount() const { return accountList.count(); }
    /**
     * Returns a copy of the Account list.
     */
    AccountList getAccountList() const { return accountList; }
    /**
     * Returns an iterator that can be used to traverse the Account list. The
     * Account list is a flat list of all accounts.
     */
    AccountListIterator getAccountListIterator() const
    {
        return AccountListIterator(accountList);
    }
    /**
     * This function adds a new, user-defined attribute to the Account 
     * attributes. The @param id must be unique within the Account attributes
     * namespace. @param cad is a pointer to the CustomAttributeDefinition
     * object.
     */
    bool addAccountAttribute(const QString& name, 
                              CustomAttributeDefinition* cad);
    /**
     * Returns a pointer to the custom attribute object identified by @param id.
     * If no attributes with the id exists, 0 is returned.
     */
    const CustomAttributeDefinition* getAccountAttribute(const QString& id)
        const;
    /**
     * Returns a read-only pointer to the dictionary that holds the pointers
     * to user-defined attributes of Accounts.
     */
    const QDict<const CustomAttributeDefinition>&
        getAccountAttributeDict() const 
    { 
        return (const QDict<const CustomAttributeDefinition>&)
            resourceAttributes; 
    }

    /**
     * This function is for library internal use only. Creating a Shift 
     * object with the project as parameter will automatically add it to the
     * Shift list of the project.
     */
    void addShift(Shift* s);
    /**
     * This function is for library internal use only. Deleting a Shift
     * will automatically delete it from the respective list of the
     * project.
     */
    void deleteShift(Shift* s);
    /**
     * Returns a pointer to the Shift with the specified ID. The ID may
     * not be an absolute ID since the Shift list has a flat namespace. If
     * no Shift with the ID exists 0 is returned.
     */
    Shift* getShift(const QString& id) const
    {
        return shiftList.getShift(id);
    }
    /**
     * Returns the number of shifts in the shift list.
     */
    uint shiftCount() const { return shiftList.count(); }
    /**
     * Returns a copy of the Shift list.
     */
    ShiftList getShiftList() const { return shiftList; }
    /**
     * Returns an iterator that can be used to traverse the Shift list. The
     * Shift list is a flat list of all accounts.
     */
    ShiftListIterator getShiftListIterator() const
    {
        return ShiftListIterator(shiftList);
    }

    /**
     * Set the minimum daily effort for resources. This value is not used for
     * resource allocation. It is currently not used at all. In future
     * versions TaskJuggler might spit out warnings for all resources that are
     * not loaded to at least this value each day. The value is inherited by
     * all resources that are created subsequent to this function call.
     */
    void setMinEffort(double m) { minEffort = m; }
    /**
     * Returns the default minimum effort value.
     */
    double getMinEffort() const { return minEffort; }

    /**
     * Set the maximum daily effort for resources. This is the default value
     * inherited by all resources created subsequent to this function call.  A
     * resource is never loaded more each day than the maximum effort value
     * specified for the resource.
     */
    void setMaxEffort(double m) { maxEffort = m; }
    /**
     * Returns the default maximum daily effort value.
     */
    double getMaxEffort() const { return maxEffort; }

    /**
     * Set the default daily cost rate for resources. This value is inherited
     * by all resources created subsequent to this function call.
     */
    void setRate(double r) { rate = r; }
    /**
     * Return the default daily cost rate for resources.
     */
    double getRate() const { return rate; }

    /**
     * Set the currency unit for the project.
     */
    void setCurrency(const QString& s) { currency = s; }
    /**
     * Returns the currency unit setting.
     */
    const QString& getCurrency() const { return currency; }

    /**
     * Sets the number of decimal digits used for all currency values of the
     * project.
     */
    void setCurrencyDigits(uint d) { currencyFormat.setFracDigits(d); }

    /**
     * Sets the default number format for load and other values.
     */
    void setNumberFormat(const RealFormat& rf) { numberFormat = rf; }
    /**
     * Returns the current default number format.
     */
    const RealFormat& getNumberFormat() const { return numberFormat; }

    /**
     * Sets the default currency format.
     */
    void setCurrencyFormat(const RealFormat& rf) { currencyFormat = rf; }
    /**
     * Returns the default currency format.
     */
    const RealFormat& getCurrencyFormat() const { return currencyFormat; }

    /**
     * Set the default time zone for the project.
     */
    void setTimeZone(const QString& tz);
    /**
     * Returns the default time zone of the project;
     */
    const QString& getTimeZone() const { return timeZone; }

    /**
     * Sets the format used for timestamps in reports. It will be used as
     * default for all subsequent report definitions. See the TaskJuggler
     * manual for the format details.
     */
    void setShortTimeFormat(const QString& tf) { shortTimeFormat = tf; }
    /**
     * Returns the format for timestamps in reports.
     */
    const QString& getShortTimeFormat() const { return shortTimeFormat; }

    /**
     * Sets the format used for date stamps in reports. It will be used as
     * default for all subsequent report definitions. See the TaskJuggler
     * manual for the format details.
     */
    void setTimeFormat(const QString& tf) { timeFormat = tf; }
    /**
     * Returns the format for date stamps in reports.
     */
    const QString& getTimeFormat() const { return timeFormat; }

    void addXMLReport(ReportXML *r ) { xmlreport = r; }

    void addReport(Report* r)
    {
        reports.append(r);
    }

    bool loadFromXML( const QString& file );
    void parseDomElem( QDomElement& parentElem );

#ifdef HAVE_ICAL
#ifdef HAVE_KDE
    void addICalReport( ReportICal *ic ) { icalReport = ic; }
#endif
#endif

    void addAllowedFlag(QString flag)
    {
        if (!isAllowedFlag(flag))
            allowedFlags.append(flag);
    }
    bool isAllowedFlag(const QString& flag) const
    {
        return allowedFlags.contains(flag) > 0;
    }

    double convertToDailyLoad(long secs) const;

    void setKotrus(Kotrus* k);
    Kotrus* getKotrus() const { return kotrus; }

    bool readKotrus();
    bool updateKotrus();

    void setAllocationErrors(bool e) { allocationErrors = e; }
    bool getAllocationErrors() const { return allocationErrors; }

    void setMaxErrors(int me) { maxErrors = me; }

    /**
     * Generate cross references between all data structures and run a
     * consistency check. This function must be called after the project data
     * tree has been contructed. 
     * @return Only if all tests were successful TRUE is returned.
     */
    bool pass2(bool noDepCheck);

    /**
     * Schedule all tasks for all scenarios. @return In case any errors were
     * detected FALSE is returned.
     */
    bool scheduleAllScenarios();
    void generateReports() const;

private:
    void overlayScenario(int base, int sc);
    void prepareScenario(int sc);
    void finishScenario(int sc);
    
    bool schedule(int sc);

    bool checkSchedule(int sc) const;

    /// The start date of the project
    time_t start;
    /// The end date of the project
    time_t end;
    /// The current date used in status calculations and reports.
    time_t now;

    bool allowRedefinitions;

    /// True if week based calculations use Monday as first day of week.
    bool weekStartsMonday;

    /// The name of the Project
    QString name;
    /// The revision of the project description.
    QString version;
    /// Some legal words to please the boss.
    QString copyright;
    
    /// The default timezone of the project.
    QString timeZone;
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

    /// The currency of used for all money values.
    QString currency;
    /// The number of fraction digits of all money values.
    int currencyDigits;

    // The default format used for printing load values and the like.
    RealFormat numberFormat;
    // The default format used for printing currency values.
    RealFormat currencyFormat;

    /**
     * The default priority that will be inherited by all tasks. Sub tasks
     * will inherit the priority of its parent task. */
    int priority;

    /// Default values for Resource variables
    double minEffort;
    double maxEffort;
    double rate;

    /* The average number of working hours per day. This factor is used
     * when converting hours in working days. It should match the workingHours
     * closely. */
    double dailyWorkingHours;

    /* The average number of working days per year. This factor is used when
     * converting working days into years. It should match the defined working
     * hours and vacation days. */
    double yearlyWorkingDays;

    /* The list of standard working or opening hours. These values will be
     * inherited by the resources as default working hours. */
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

    QString currentId;

    bool allocationErrors;

    int maxErrors;

    VacationList vacationList;

    ScenarioList scenarioList;  
    TaskList taskList;
    ResourceList resourceList;
    AccountList accountList;
    ShiftList shiftList;

    QDict<CustomAttributeDefinition> taskAttributes;
    QDict<CustomAttributeDefinition> resourceAttributes;

    Kotrus* kotrus;

    ReportXML* xmlreport;
#ifdef HAVE_ICAL
#ifdef HAVE_KDE
    ReportICal *icalReport;
#endif
#endif

    QPtrList<Report> reports;   
} ;

#endif
