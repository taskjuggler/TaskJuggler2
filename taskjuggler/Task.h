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

#include <stdarg.h>

#include <qlist.h>
#include <qdict.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qdom.h>
#include <time.h>
#include "config.h"
#include "ResourceList.h"
#include "Utility.h"
#include "CoreAttributes.h"
#include "ShiftList.h"

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
	TaskList() { }
	virtual ~TaskList() { }

	Task* first() { return (Task*) CoreAttributesList::first(); }
	Task* next() { return (Task*) CoreAttributesList::next(); }

	Task* getTask(const QString& id);

protected:
	virtual int compareItems(QCollection::Item i1, QCollection::Item i2);
} ;

typedef QPtrListIterator<TaskList> TaskListIterator;



class Task : public CoreAttributes
{
	friend int TaskList::compareItems(QCollection::Item i1,
									  QCollection::Item i2);

public:
	Task(Project* prj, const QString& id_, const QString& n, Task* p,
		 const QString& f, int l);
	virtual ~Task() { }

	virtual const char* getType() { return "Task"; }

	enum SchedulingInfo { ASAP, ALAP };

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

	void setComplete(int c) { complete = c; }
	double getComplete() const { return complete; }

	void setStartBuffer(double p) { startBuffer = p; }
	double getStartBuffer() const { return startBuffer; }
	
	void setEndBuffer(double p) { endBuffer = p; }
	double getEndBuffer() const { return endBuffer; }

	void setResponsible(Resource* r) { responsible = r; }
	Resource* getResponsible() const { return responsible; }

	void addDependency(const QString& id) { dependsIds.append(id); }
	void addPreceeds(const QString& id) { preceedsIds.append(id); }

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

	/* The following group of functions operates exclusively on 'plan'
	 * variables. */
	void setPlanStart(time_t s) { planStart = s; }
	const time_t getPlanStart() const { return planStart; }

	void setPlanEnd(time_t s) { planEnd = s; }
	const time_t getPlanEnd() const { return planEnd; }

	time_t getPlanStartBufferEnd() const { return planStartBufferEnd; }
	time_t getPlanEndBufferStart() const { return planEndBufferStart; }

	void setPlanLength(double days) { planLength = days; }
	double getPlanLength() const { return planLength; }

	void setPlanEffort(double e) { planEffort = e; }
	double getPlanEffort() const { return planEffort; }

	void setPlanDuration(double d) { planDuration = d; }
	double getPlanDuration() const { return planDuration; }

	bool isPlanStartOk()
	{
		return planStart >= minStart && planStart <= maxStart;
	}
	bool isPlanEndOk()
	{
		return planEnd >= minEnd && planEnd <= maxEnd;
	}

	bool isPlanBuffer(const Interval& iv) const
	{
		return iv.overlaps(Interval(planStart, planStartBufferEnd)) ||
			iv.overlaps(Interval(planEndBufferStart, planEnd));
	}
	
	double getPlanCalcEffort()
	{
		return getPlanLoad(Interval(planStart, planEnd));
	}
	double getPlanCalcDuration() const;

	double getPlanCredits(const Interval& period, Resource* resource = 0,
						  bool recursive = TRUE);

	bool isPlanActive(const Interval& period) const;

	double getPlanLoad(const Interval& period, Resource* resource = 0);

	void addPlanBookedResource(Resource* r)
	{
		if (planBookedResources.find(r) == -1)
			planBookedResources.inSort(r);
	}
	bool isPlanBookedResource(Resource* r)
	{
		return planBookedResources.find(r) != -1;
	}
	QPtrList<Resource> getPlanBookedResources() { return planBookedResources; }

	/* The following group of functions operates exclusively on 'actual'
	 * variables. */
	void setActualStart(time_t s) { actualStart = s; }
	time_t getActualStart() const { return actualStart; }

	void setActualEnd(time_t e) { actualEnd = e; }
	time_t getActualEnd() const { return actualEnd; }

	time_t getActualStartBufferEnd() const { return actualStartBufferEnd; }
	time_t getActualEndBufferStart() const { return actualEndBufferStart; }

	void setActualLength(double days) { actualLength = days; }
	double getActualLength() const { return actualLength; }

	void setActualEffort(double e) { actualEffort = e; }
	double getActualEffort() const { return actualEffort; }

	void setActualDuration(double d) { actualDuration = d; }
	double getActualDuration() const { return actualDuration; }

	bool isActualStartOk()
	{
		return actualStart >= minStart && actualStart <= maxStart;
	}
	bool isActualEndOk()
	{
		return actualEnd >= minEnd && actualEnd <= maxEnd;
	}

	bool isActualBuffer(const Interval& iv) const
	{
		return iv.overlaps(Interval(actualStart, actualStartBufferEnd)) ||
			iv.overlaps(Interval(actualEndBufferStart, actualEnd));
	}
	
	double getActualCalcEffort()
	{
		return getActualLoad(Interval(actualStart, actualEnd));
	}
	double getActualCalcDuration() const;

	double getActualCredits(const Interval& period, Resource* resource = 0,
							bool recursive = TRUE);

	bool isActualActive(const Interval& period) const;

	double getActualLoad(const Interval& period, Resource* r = 0);
	void addActualBookedResource(Resource* r)
	{
		if (actualBookedResources.find(r) == -1)
			actualBookedResources.inSort(r);
	}
	bool isActualBookedResource(Resource* r)
	{
		return actualBookedResources.find(r) != -1;
	}

	QPtrList<Resource> getActualBookedResources()
	{
		return actualBookedResources;
	}

	bool isContainer() const { return !sub.isEmpty(); }
   
	bool xRef(QDict<Task>& hash);
	QString resolveId(QString relId);
	bool schedule(time_t& reqStart, time_t duration);
	bool isScheduled() const { return schedulingDone; }
	void setScheduled() { schedulingDone = TRUE; }
	bool needsEarlierTimeSlot(time_t date);
	void propagateStart(bool safeMode = TRUE);
	void propagateEnd(bool safeMode = TRUE);
	void propagateInitialValues();

	/**
	 * @returns TRUE if the work planned for a day has been completed.
	 * This is either specified by the 'complete' attribute or if no
	 * complete attribute is specified, the day is completed if it has
	 * passed. This function operates on the actual start and end dates.
	 *
	 * @param date specifies the day that should be checked.
	 */
	bool isCompleted(time_t date) const;
	double getCompleteAtTime(time_t) const;
	bool scheduleOk();
	bool preScheduleOk();
	void computeBuffers();
	bool isActive();

	void setMilestone() { milestone = TRUE; }
	bool isMilestone() const { return milestone; }

	void setAccount(Account* a) { account = a; }
	Account* getAccount() const { return account; }

	void setStartCredit(double c) { startCredit = c; }
	double getStartCredit() const { return startCredit; }

	void setEndCredit(double c) { endCredit = c; }
	double getEndCredit() const { return endCredit; }

	void getSubTaskList(TaskList& tl);

	bool isSubTask(Task* t);

	void treeSortKey(QString& key);

	void preparePlan();
	void finishPlan();

	void prepareActual();
	void finishActual();
	bool hasActualValues() const
	{
		return actualStart != 0 || actualEnd != 0 || actualLength != 0 ||
			actualDuration != 0 || actualEffort != 0;
	}

	QDomElement xmlElement( QDomDocument& doc, bool absId = true );

	static void setDebugLevel(int l) { debugLevel = l; }

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
	void fatalError(const char* msg, ...) const;

	/// A longer description of the task.
	QString note;
	
	/**
	 * List of tasks Ids that need to be completed before this task
	 * can start. */
	QStringList dependsIds;

	/// A list of task pointers created from dependsIds in xRef.
	TaskList depends;

	/// List of tasks Ids that have to follow when this task is completed.
	QStringList preceedsIds;

	/// A list of task pointers created from preceedsIds in xRef.
	TaskList preceeds;

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

	/// Percentage of completion of the task
	int complete;

	/* Specifies how many percent the task start can be delayed but still
	 * finish in time if all goes well. This value is for documentation
	 * purposes only. It is not used for task scheduling. */
	double startBuffer;

	/* Specifies how many percent the task can be finished earlier if
	 * all goes well. This value is for documentation purposes only. It is
	 * not used for task scheduling. */
	double endBuffer;
	
	/// ID of responsible resource.
	Resource* responsible;

	/// Tasks may only be worked on during the specified shifts.
	ShiftSelectionList shifts;

	/// List of resource allocations requested by the task
	QPtrList<Allocation> allocations;

	/// Account where the credits of the task are credited to.
	Account* account;

	/// Amount that is credited to the account at the start date.
	double startCredit;
	/// Amount that is credited to the account at the end date.
	double endCredit;

	/* The following group of variables store plan values. Their
	 * values are copied to the runtime equivalents (without the
	 * 'plan' prefix) before the 'plan' scheduling is done. */

	/// Time when the task is planned to start.
	time_t planStart;

	/// Time when the task is planned to finish.
	time_t planEnd;

	/// Time when the start buffer is planned to end.
	time_t planStartBufferEnd;
	
	/// Time when the end buffer is planned to start.
	time_t planEndBufferStart;
	
	/// The planned duration of the task (in calendar days).
	double planDuration;

	/// The planned length of the task (in working days).
	double planLength;

	/// The planned effort of the task (in resource-days).
	double planEffort;

	/// List of booked resources for the 'plan' scenario.
	QPtrList<Resource> planBookedResources;

	/* The following group of variables store actual values. Their
	 * values are copied to the runtime equivalents (without the
	 * 'actual' prefix) before the 'actual' scheduling is done. */

	/// Time when it really started
	time_t actualStart;

	/// Time when it really ended
	time_t actualEnd;

	/// Time when the start buffer actually ends.
	time_t actualStartBufferEnd;

	/// Time when the end buffer actually starts.
	time_t actualEndBufferStart;
	
	/// The actual duration of the task (in calendar days).
	double actualDuration;

	/// The actual length of the task (in working days).
	double actualLength;

	/// The actual Effort of the task (in resource-days).
	double actualEffort;

	/// List of booked resources for the 'actual' scenario.
	QPtrList<Resource> actualBookedResources;

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

	/// A list of all the resources booked for this task.
	QPtrList<Resource> bookedResources;

	static int debugLevel;
} ;

#endif
