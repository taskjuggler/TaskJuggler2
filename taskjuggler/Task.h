/*
 * task.h - TaskJuggler
 *
 * Copyright (c) 2001 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _Task_h_
#define _Task_h_

#include <qlist.h>
#include <qdict.h>
#include <qstring.h>
#include <qstringlist.h>
#include <time.h>

#include "ResourceList.h"
#include "Utility.h"

class Project;
class Resource;
class Account;

class Allocation
{
public:
	Allocation(Resource* r) : resource(r)
	{
		alternatives.setAutoDelete(TRUE);
		load = 100;
		persistent = FALSE;
	}
	~Allocation() { }

	Resource* getResource() const { return resource; }

	void setLoad(int l) { load = l; }
	int getLoad() const { return load; }

	void setPersistent(bool p) { persistent = p; }
	bool getPersistent() const { return persistent; }

	void addAlternative(Resource* r) { alternatives.append(r); }
	Resource* first() { return alternatives.first(); }
	Resource* next() { return alternatives.next(); }

private:
	// Don't use this.
	Allocation();

	Resource* resource;
	// The maximum daily usage of the resource.
	int load;

	/* True if the allocation should be persistent over the whole task.
	 * If set the first selection will not be changed even if there is an
	 * available alternative. */
	bool persistent;

	// List of alternative resources.
	QList<Resource> alternatives;
} ;

class Task
{
public:
	Task(Project* prj, const QString& id_, const QString& n, Task* p,
		 const QString f, int l);
	~Task() { }

	const QString& getId() const { return id; }

	void setName(const QString& n) { name = n; }
	const QString& getName() const { return name; }

	void setNote(const QString& d) { note = d; }
	const QString& getNote() const { return note; }

	void setPriority(int p) { priority = p; }
	int getPriority() const { return priority; }

	void setStart(time_t s) { start = s; }
	const time_t getStart() const { return start; }
	const QString getStartISO() const { return time2ISO(start); }

	void setEnd(time_t s) { end = s; }
	const time_t getEnd() const { return end; }
	const QString getEndISO() const { return time2ISO(end); }

	void setMinStart(time_t s) { minStart = s; }
	time_t getMinStart() const { return minStart; }

	void setMaxStart(time_t s) { maxStart = s; }
	time_t getMaxStart() const { return maxStart; }

	void setMinEnd(time_t e) { minEnd = e; }
	time_t getMinEnd() const { return minEnd; }

	void setMaxEnd(time_t e) { maxEnd = e; }
	time_t getMaxEnd() const { return maxEnd; }

	void setActualStart(time_t s) { actualStart = s; }
	time_t getActualStart() const { return actualStart; }

	void setActualEnd(time_t e) { actualEnd = e; }
	time_t getActualEnd() const { return actualEnd; }

	bool isStartOk()
	{
		return start >= minStart && start <= maxStart;
	}
	bool isEndOk()
	{
		return end >= minEnd && end <= maxEnd;
	}

	void setLength(int days) { length = days; }
	double getLength() const { return length; }

	void setEffort(double e) { effort = e; }
	double getEffort() const { return effort; }

	void setDuration(double d) { duration = d; }
	double getDuration() const { return duration; }

	void setComplete(int c) { complete = c; }
	double getComplete() const { return complete; }

	void addDependency(const QString& id) { depends.append(id); }
	Task* firstPrevious() { return previous.first(); }
	Task* nextPrevious() { return previous.next(); }

	void addFollower(Task* t) { followers.append(t); }
	Task* firstFollower() { return followers.first(); }
	Task* nextFollower() { return followers.next(); }

	void addAllocation(Allocation* a) { allocations.append(a); }
	Allocation* firstAllocation() { return allocations.first(); }
	Allocation* nextAllocation() { return allocations.next(); }

	double getLoadOnDay(time_t day);
	void addBookedResource(Resource* r)
	{
		if (bookedResources.find(r) == -1)
			bookedResources.inSort(r);
	}
	Resource* firstBookedResource() { return bookedResources.first(); }
	Resource* nextBookedResource() { return bookedResources.next(); }

	Task* getParent() const { return parent; }
	void addSubTask(Task* t) { subTasks.append(t); }

	bool xRef(QDict<Task>& hash);
	QString resolveId(QString relId);
	bool schedule(time_t reqStart, time_t duration);
	bool isScheduled();
	bool scheduleOK();

	bool isMilestone() const { return start != 0 && start == end; }
	bool isActiveToday(time_t date) const
	{
		Interval day(midnight(date), midnight(date) + ONEDAY - 1);
		Interval work(start, end);
		return day.overlap(work);
	}

	void setAccount(Account* a) { account = a; }

	void addFlag(QString flag)
	{
		if (!hasFlag(flag))
			flags.append(flag);
	}
	void clearFlag(const QString& flag)
	{
		flags.remove(flag);
	}
	bool hasFlag(const QString& flag)
	{
		return flags.contains(flag) > 0;
	}

private:
	bool scheduleContainer();
	bool bookResource(Resource* r, time_t day, time_t duration);
	bool bookResources(time_t day, time_t duration);
	bool isWorkingDay(time_t day) const;
	time_t nextWorkingDay(time_t day) const;
	time_t earliestStart();
	void fatalError(const QString& msg) const;

	Project* project;
	// The task Id. Must be unique in a project.
	QString id;
	// The task name. A short description.
	QString name;
	// A longer description.
	QString note;
	
	// Pointer to parent task.
	Task* parent;
	// List of sub tasks.
	QList<Task> subTasks;

	// Name if the file where this task has been defined.
	QString file;
	// Line in the file where the task definition starts
	int line;

	/* The priority is used during scheduling. The higher the priority the
	 * more likely the task will get the requested resources. */
	int priority;

	// Day when the task should start
	time_t start;
	// Day when the task should end
	time_t end;
	// Ealiest day when the task should start
	time_t minStart;
	// Latest day when the task should start
	time_t maxStart;
	// Ealiest day when the task should end
	time_t minEnd;
	// Latest day when the task should end
	time_t maxEnd;
	// Day when it really started
	time_t actualStart;
	// Day when it really ended
	time_t actualEnd;
	// Length in working days
	double length;
	// Effort (in man days) needed to complete the task
	double effort;
	// Duration in calender days
	double duration;

	// Percentage of completion of the task
	int complete;

	// The following block of variables are used during scheduling.
	double costs;
	double doneEffort;
	double doneLength;
	double doneDuration;
	bool workStarted;
	time_t tentativeEnd;
	time_t lastSlot;

	// Account where the costs of the task are credited to.
	Account* account;

	// A list of flags that can be used to select a sub-set of tasks.
	QStringList flags;

	// List of tasks Ids that need to be completed before this task can start
	QStringList depends;
	// Same as previous but pointers to tasks that are resolved in pass2
	QList<Task> previous;
	// List of tasks that depend on this task
	QList<Task> followers;
	// List of resource allocations requested by the task
	QList<Allocation> allocations;
	// List of booked resources
	QList<Resource> bookedResources;
} ;

class TaskList : public QList<Task>
{
public:
	TaskList();
	virtual ~TaskList();

	enum SortCriteria { PrioUp, PrioDown };

	void setSorting(SortCriteria s) { sorting = s; }

protected:
	virtual int compareItems(QCollection::Item i1, QCollection::Item i2);

private:
	SortCriteria sorting;
} ;

#endif
