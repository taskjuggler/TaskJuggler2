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

class Allocation
{
public:
	Allocation(Resource* r) : resource(r)
	{
		alternatives.setAutoDelete(TRUE);
		load = 100;
	}
	~Allocation() { }

	Resource* getResource() const { return resource; }

	void setLoad(int l) { load = l; }
	int getLoad() const { return load; }

	void addAlternative(Resource* r) { alternatives.append(r); }
	Resource* first() { return alternatives.first(); }
	Resource* next() { return alternatives.next(); }

private:
	// Don't use this.
	Allocation();

	Resource* resource;
	int load;
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

	void setStart(time_t s) { start = s; }
	const time_t getStart() const { return start; }
	const QString getStartISO() const { return time2ISO(start); }

	void setEnd(time_t s) { end = s; }
	const time_t getEnd() const { return end; }
	const QString getEndISO() const { return time2ISO(end); }

	void setMinStart(time_t s) { minStart = s; }
	void setMaxStart(time_t s) { maxStart = s; }
	void setMinEnd(time_t e) { minEnd = e; }
	void setMaxEnd(time_t e) { maxEnd = e; }
	void setActualStart(time_t s) { actualStart = s; }
	void setActualEnd(time_t e) { actualEnd = e; }

	void setLength(int days) { length = days; }
	int getLength() const { return length; }

	void setEffort(double e) { effort = e; }
	double getEffort() const { return effort; }

	void setComplete(int c) { complete = c; }
	double getComplete() const { return complete; }

	void addDependency(const QString& id) { depends.append(id); }
	void addFollower(Task* t) { followers.append(t); }

	void addAllocation(Allocation* a) { allocations.append(a); }

	Task* getParent() const { return parent; }
	void addSubTask(Task* t) { subTasks.append(t); }

	bool xRef(QDict<Task>& hash);
	bool schedule(time_t reqStart);
	void scheduleContainer();
	bool isScheduled();
	bool scheduleOK();

private:
	bool bookResource(time_t day, bool& workStarted, double& done);
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
	int length;
	// Effort (in man days) needed to complete the task
	double effort;
	// Percentage of completion of the task
	int complete;

	// List of tasks Ids that need to be completed before this task can start
	QStringList depends;
	// Same as previous but pointers to tasks that are resolved in pass2
	QList<Task> previous;
	// List of tasks that depend on this task
	QList<Task> followers;
	// List of resource allocations requested by the task
	QList<Allocation> allocations;
} ;

#endif






