/*
 * task.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _Task_h_
#define _Task_h_

#include <config.h>

#include <stdarg.h>

#include <qlist.h>
#include <qdict.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qdom.h>
#include <time.h>

#include "TaskScenario.h"
#include "ResourceList.h"
#include "Utility.h"
#include "CoreAttributes.h"
#include "ShiftList.h"
#include "LoopDetectorInfo.h"

#ifdef HAVE_ICAL
#ifdef HAVE_KDE
#include <libkcal/todo.h>
#include <libkcal/calendarlocal.h>
#endif
#endif

class Project;
class Resource;
class Account;
class TaskList;
class QDomElement;
class QDomDocument;
class Task;
class Allocation;

class TaskList : public virtual CoreAttributesList
{
public:
	TaskList()
   	{
	   	sorting[0] = CoreAttributesList::TreeMode;
		sorting[1] = CoreAttributesList::PlanStartUp;
		sorting[2] = CoreAttributesList::PlanEndUp;
	}
	virtual ~TaskList() { }

	Task* first() { return (Task*) CoreAttributesList::first(); }
	Task* last()  { return (Task*) CoreAttributesList::last(); }
	Task* prev()  { return (Task*) CoreAttributesList::prev(); }
	Task* next()  { return (Task*) CoreAttributesList::next(); }

	Task* getTask(const QString& id);

	static bool isSupportedSortingCriteria
		(CoreAttributesList::SortCriteria sc);
	
	virtual int compareItemsLevel(Task* t1, Task* T2, int level);

protected:
	virtual int compareItems(QCollection::Item i1, QCollection::Item i2);
} ;

typedef QPtrListIterator<TaskList> TaskListIterator;


class Task : public CoreAttributes
{
	friend int TaskList::compareItemsLevel(Task*, Task*, int);

public:
	Task(Project* prj, const QString& id_, const QString& n, Task* p,
		 const QString& f, int l);
	virtual ~Task() { }

	virtual const char* getType() { return "Task"; }

	enum SchedulingInfo { ASAP, ALAP };

	enum Scenario { Plan = 0, Actual };

	Task* getParent() { return (Task*) parent; }

	void setProjectId(const QString& i) { projectId = i; }
	const QString& getProjectId() const { return projectId; }

	void setNote(const QString& d) { note = d; }
	const QString& getNote() const { return note; }

	void setScheduling(SchedulingInfo si) { scheduling = si; }
	SchedulingInfo getScheduling() const { return scheduling; }

	void setPriority(int p) { priority = p; }
	int getPriority() const { return priority; }

	void setMinStart(time_t s) { minStart = s; }
	time_t getMinStart() const { return minStart; }

	void setMaxStart(time_t s) { maxStart = s; }
	time_t getMaxStart() const { return maxStart; }

	void setMinEnd(time_t e) { minEnd = e; }
	time_t getMinEnd() const { return minEnd; }

	void setMaxEnd(time_t e) { maxEnd = e; }
	time_t getMaxEnd() const { return maxEnd; }

	void setResponsible(Resource* r) { responsible = r; }
	Resource* getResponsible() const { return responsible; }

	void setMilestone() { milestone = TRUE; }
	bool isMilestone() const { return milestone; }

	void setAccount(Account* a) { account = a; }
	Account* getAccount() const { return account; }

	bool addDepends(const QString& id);
	bool addPrecedes(const QString& id);

	bool addShift(const Interval& i, Shift* s)
	{
		return shifts.insert(new ShiftSelection(i, s));
	}

	void addAllocation(Allocation* a) { allocations.append(a); }
	Allocation* firstAllocation() { return allocations.first(); }
	Allocation* nextAllocation() { return allocations.next(); }

	Task* firstPrevious() { return previous.first(); }
	Task* nextPrevious() { return previous.next(); }

	Task* firstFollower() { return followers.first(); }
	Task* nextFollower() { return followers.next(); }

	bool hasPrevious(Task* t) { return previous.find(t) != -1; }
	bool hasFollower(Task* t) { return followers.find(t) != -1; }

	// The following group of functions operates only on scenario variables.
	void setStart(int sc, time_t s) { scenarios[sc].start = s; }
	const time_t getStart(int sc) const { return scenarios[sc].start; }

	void setEnd(int sc, time_t s) { scenarios[sc].end = s; }
	const time_t getEnd(int sc) const { return scenarios[sc].end; }

	time_t getStartBufferEnd(int sc) const 
	{
	   	return scenarios[sc].startBufferEnd; 
	}
	time_t getEndBufferStart(int sc) const 
	{ 
		return scenarios[sc].endBufferStart; 
	}

	void setLength(int sc, double days) { scenarios[sc].length = days; }
	double getLength(int sc) const { return scenarios[sc].length; }

	void setEffort(int sc, double e) { scenarios[sc].effort = e; }
	double getEffort(int sc) const { return scenarios[sc].effort; }

	void setDuration(int sc, double d) { scenarios[sc].duration = d; }
	double getPlanDuration(int sc) const { return scenarios[sc].duration; }

	bool isStartOk(int sc)
	{
		return (minStart <= scenarios[sc].start && 
				scenarios[sc].start <= maxStart);
	}
	bool isEndOk(int sc)
	{
		return (minEnd <= scenarios[sc].end + (milestone ? 1 : 0) &&
				scenarios[sc].end + (milestone ? 1 : 0) <= maxEnd);
	}

	bool isBuffer(int sc, const Interval& iv) const
	{
		return iv.overlaps(Interval(scenarios[sc].start,
								   	scenarios[sc].startBufferEnd)) ||
			iv.overlaps(Interval(scenarios[sc].endBufferStart, 
								 scenarios[sc].end));
	}
	
	void setComplete(int sc, int c) { scenarios[sc].complete = c; }
	double getComplete(int sc) const { return scenarios[sc].complete; }

	void setStartBuffer(int sc, double p) { scenarios[sc].startBuffer = p; }
	double getStartBuffer(int sc) const { return scenarios[sc].startBuffer; }
	
	void setEndBuffer(int sc, double p) { scenarios[sc]. endBuffer = p; }
	double getEndBuffer(int sc) const { return scenarios[sc].endBuffer; }

	void setStartCredit(int sc, double c) { scenarios[sc].startCredit = c; }
	double getStartCredit(int sc) const { return scenarios[sc].startCredit; }

	void setEndCredit(int sc, double c) { scenarios[sc].endCredit = c; }
	double getEndCredit(int sc) const { return scenarios[sc].endCredit; }

	double getCalcEffort(int sc)
	{
		return getLoad(sc, Interval(scenarios[sc].start, scenarios[sc].end));
	}
	double getCalcDuration(int sc) const;

	double getCredits(int sc, const Interval& period, 
					  Resource* resource = 0, bool recursive = TRUE);

	bool isActive(int sc, const Interval& period) const;
	bool isCompleted(int sc, time_t date) const;
	double getCompleteAtTime(int sc, time_t) const;

	double getLoad(int sc, const Interval& period, Resource* resource = 0);

	void addBookedResource(int sc, Resource* r)
	{
		if (scenarios[sc].bookedResources.find(r) == -1)
			scenarios[sc].bookedResources.inSort(r);
	}
	bool isBookedResource(int sc, Resource* r)
	{
		return scenarios[sc].bookedResources.find(r) != -1;
	}
	QPtrList<Resource> getBookedResources(int sc) 
	{
		return scenarios[sc].bookedResources; 
	}
	void setScheduled(int sc, bool ps) { scenarios[sc].scheduled = ps; }
	bool getScheduled(int sc) const { return scenarios[sc].scheduled; }

	void overlayScenario(int sc);
	void prepareScenario(int sc);
	void finishScenario(int sc);

	bool hasExtraValues(int sc) const;

	bool isContainer() const { return !sub.isEmpty(); }
   
	bool xRef(QDict<Task>& hash);
	void implicitXRef();
	QString resolveId(QString relId);
	void schedule(time_t& reqStart, time_t duration);
	bool isScheduled() const { return schedulingDone; }
	void propagateStart(bool safeMode = TRUE);
	void propagateEnd(bool safeMode = TRUE);
	void propagateInitialValues();
	void setRunaway();
	bool isRunaway();

	/**
	 * @returns TRUE if the work planned for a day has been completed.
	 * This is either specified by the 'complete' attribute or if no
	 * complete attribute is specified, the day is completed if it has
	 * passed. This function operates on the actual start and end dates.
	 *
	 * @param date specifies the day that should be checked.
	 */
	bool scheduleOk(int& errors, QString scenario);
	bool preScheduleOk();
	bool loopDetector();
	bool loopDetection(LDIList list, bool atEnd, bool fromSub, bool fromParent);
	void computeBuffers();
	bool isActive();
	time_t nextSlot(time_t slotDuration);

	void getSubTaskList(TaskList& tl);

	bool isSubTask(Task* t);

	void fatalError(const char* msg, ...) const;

	QDomElement xmlElement( QDomDocument& doc, bool absId = true );

	static void setDebugLevel(int l) { debugLevel = l; }
	static void setDebugMode(int m) { debugMode = m; }

#ifdef HAVE_ICAL
#ifdef HAVE_KDE
   void toTodo( KCal::Todo *, KCal::CalendarLocal * );
#endif
#endif
   void loadFromXML( QDomElement& parent, Project *project );
   
private:
	bool scheduleContainer(bool safeMode);
	Task* subFirst() { return (Task*) sub.first(); }
	Task* subNext() { return (Task*) sub.next(); }
	bool bookResource(Resource* r, time_t day, time_t duration,
					  int loadFactor);
	bool bookResources(time_t day, time_t duration);
	void addBookedResource(Resource* r)
	{
		if (bookedResources.find(r) == -1)
			bookedResources.inSort(r);
	}
	QPtrList<Resource> createCandidateList(time_t date, Allocation* a);
	time_t earliestStart();
	time_t latestEnd();

	bool hasStartDependency(int sc);
	bool hasEndDependency(int sc);

	/// A longer description of the task.
	QString note;
	
	/**
	 * List of tasks Ids that need to be completed before this task
	 * can start. */
	QStringList dependsIds;

	/// A list of task pointers created from dependsIds in xRef.
	TaskList depends;

	/// List of tasks Ids that have to follow when this task is completed.
	QStringList precedesIds;

	/// A list of task pointers created from preceedsIds in xRef.
	TaskList precedes;

	/**
	 * A list of all tasks that preceed this task. This is redundant
	 * information but stored for conveniance. Interdependent tasks are
	 * linked in a doubly linked list. */
	TaskList previous;

	/**
	 * A list of all tasks that follow this task. This is redundant
	 * information but stored for conveniance. Interdependent tasks are
	 * linked in a doubly linked list. */
	TaskList followers;

	/**
	 * The ID of the project this task belongs to. This is only
	 * meaningful if multiple projects are joined to create a big
	 * project. */
	QString projectId;

	/**
	 * Name of the file where this task has been defined. This is used
	 * for error reports. */
	QString file;

	/**
	 * Line in the file where the task definition starts. This is used
	 * for error reports. */
	int line;

	/// True if the task is a milestone.
	bool milestone;

	/**
	 * The priority is used during scheduling. The higher the priority
	 * the more likely the task will get the requested resources. */
	int priority;

	/// Ealiest day when the task should start
	time_t minStart;

	/// Latest day when the task should start
	time_t maxStart;

	/// Ealiest day when the task should end
	time_t minEnd;

	/// Latest day when the task should end
	time_t maxEnd;

	/// The scheduling policy of the task.
	SchedulingInfo scheduling;

	/// ID of responsible resource.
	Resource* responsible;

	/// Tasks may only be worked on during the specified shifts.
	ShiftSelectionList shifts;

	/// List of resource allocations requested by the task
	QPtrList<Allocation> allocations;

	/// Account where the credits of the task are credited to.
	Account* account;

	TaskScenario scenarios[2];
	
	/* The following group of variables store values generated during a
	 * scheduler run. They might be initialized by other values and/or
	 * they might contain results of the scheduling run. But they should
	 * never be initialized directly or read out directly. They should have
	 * corresponding variables which have a plan or acutal prefix that are
	 * used to initialize them or to store there values. The get/set
	 * interface functions should only access the plan/actual variables. */

	/// Day when the task should start
	time_t start;

	/// Day when the task should end
	time_t end;

	/// Length in working days
	double length;

	/// Effort (in man days) needed to complete the task
	double effort;

	/// Duration in calender days
	double duration;

	/// The already completed effort in a scheduler run.
	double doneEffort;

	/// The already completed length in a scheduler run.
	double doneLength;

	/// The already completed duration in a scheduler run.
	double doneDuration;

	/**
	 * Set to TRUE when the first time slots have with resource usage
	 * have been allocated. */
	bool workStarted;

	/**
	 * Since the full time slot might not be available we need to
	 * store the tentative start of a task in a seperate
	 * variable. Storing the information in 'start' would mark the
	 * task as fully scheduled which might not yet be the case. */
	time_t tentativeStart;

	/**
	 * Since the full time slot might not be available we need to
	 * store the tentative end of a task in a seperate
	 * variable. Storing the information in 'end' would mark the task
	 * as fully scheduled which might not yet be the case. */
	time_t tentativeEnd;

	/**
	 * Depending on the scheduling policy the tasks need to be scheduled
	 * from one end the other in a continuous way. No timeslot may be
	 * scheduled twice. This variable stores information about the last
	 * allocation, so we can make sure the next slot is exactly adjacent
	 * the the previous one. */
	time_t lastSlot;

	/// This variable is set to true when the task has been scheduled.
	bool schedulingDone;

	/** This flag is set when the task does not fit into the project time
	 * frame. */
	bool runAway;

	/// A list of all the resources booked for this task.
	QPtrList<Resource> bookedResources;

	static int debugLevel;
	static int debugMode;
} ;

#endif
