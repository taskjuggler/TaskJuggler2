/*
 * task.cpp - TaskJuggler
 *
 * Copyright (c) 2001 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

/* -- DTD --
 <!-- Task element, child of projects and for subtasks -->
 <!ELEMENT Task		(TaskName, Priority, start, end, minStart, maxStart,
                         minEnd, maxEnd, actualStart, actualEnd,
			 SubTasks*, Depends*, Previous*, Followers*,
			 Allocations*, bookedResources*, note*)>
 <!ELEMENT TaskName     (#PCDATA)>
 <!ELEMENT Priority     (#PCDATA)>
 <!ELEMENT start        (#PCDATA)>
 <!ELEMENT end          (#PCDATA)>
 <!ELEMENT minStart     (#PCDATA)>
 <!ELEMENT maxStart     (#PCDATA)>
 <!ELEMENT minEnd       (#PCDATA)>
 <!ELEMENT maxEnd       (#PCDATA)>
 <!ELEMENT actualStart  (#PCDATA)>
 <!ELEMENT actualEnd    (#PCDATA)>
 <!ELEMENT SubTasks     (Task+)>
 <!ELEMENT Depends      (TaskID+)>
 <!ELEMENT TaskID       (#PCDATA)>
 <!ELEMENT Previous     (TaskID+)>
 <!ELEMENT Followers    (TaskID+)>
 <!ELEMENT Allocations  (Allocation+)>
 <!ELEMENT Allocation   EMPTY>
 <!ELEMENT bookedResources (ResourceID+)>
 <!ELEMENT ResourceID   (#PCDATA)>
 <!ELEMENT note         (#PCDATA)>
 <!ATTLIST ResourceID
           Name CDATA #REQUIRED>
 <!ATTLIST Allocation
           load CDATA #REQUIRED
	   ResourceID CDATA #REQUIRED>
   /-- DTD --/
*/
 
#include <stdio.h>

#include "Task.h"
#include "Project.h"

Task::Task(Project* proj, const QString& id_, const QString& n, Task* p,
		   const QString f, int l)
	: FlagList(p ? *p : FlagList()), project(proj), id(id_), name(n),
	  parent(p), file(f), line(l)
{
	start = end = 0;
	scheduling = ASAP;
	actualStart = actualEnd = 0;
	length = 0.0;
	effort = 0.0;
	duration = 0.0;
	complete = -1;
	note = "";
	account = 0;
	lastSlot = 0;
	planCosts = 0.0;
	if (p)
	{
		// Set attributes that are inherited from parent task.
		priority = p->priority;
		minStart = p->minStart;
		maxStart = p->maxStart;
		minEnd = p->minEnd;
		maxEnd = p->maxEnd;
	}
	else
	{
		// Set attributes that are inherited from global attributes.
		priority = proj->getPriority();
		minStart = minEnd = proj->getStart();
		maxStart = maxEnd = proj->getEnd();
	}
}

void
Task::fatalError(const QString& msg) const
{
	qWarning("%s:%d:%s\n", file.latin1(), line, msg.latin1());
}

bool
Task::schedule(time_t date, time_t slotDuration)
{
	// Task is already scheduled.
	if (start != 0 && end != 0)
		return TRUE;

	/* Check whether this task is a container tasks (task with sub-tasks).
	 * Container tasks are scheduled when all sub tasks have been
	 * scheduled. */
	if (!subTasks.isEmpty())
		return scheduleContainer();

	if (scheduling == Task::ASAP)
	{
		bool startChanged = FALSE;
		if (start == 0)
		{
			/* No start time has been specified. The start time is
			 * either the project start time if the tasks has no
			 * previous tasks, or the start time is determined by the
			 * end date of the last previous task. */
			if (depends.count() == 0)
			{
				start = project->getStart();
				startChanged = TRUE;
				lastSlot = start - 1;
			}
			else if (earliestStart() > 0)
			{
				start = earliestStart();
				startChanged = TRUE;
				lastSlot = start - 1;
				doneEffort = 0.0;
				doneDuration = 0.0;
				doneLength = 0.0;
				planCosts = 0.0;
				workStarted = FALSE;
				tentativeStart = date;
			}
			else
				return TRUE;	// Task cannot be scheduled yet.
		}
		else if (lastSlot == 0)
			lastSlot = start - 1;
		/* Do not schedule anything if the time slot is not directly
		 * following the time slot that was previously scheduled. */
		if (date != lastSlot + 1)
			return !startChanged;
		lastSlot = date + slotDuration - 1;

	}
	else if (scheduling == Task::ALAP)
	{
		bool endChanged = FALSE;
		if (end == 0)
		{
			/* No end time has been specified. The end time is either
			 * the project end time if the tasks has no following
			 * tasks, or the end time is determined by the start date
			 * of the earliest following task. */
			if (preceeds.count() == 0)
			{
				end = project->getEnd();
				endChanged = TRUE;
				lastSlot = end + 1;
			}
			else if (latestEnd() > 0)
			{
				end = latestEnd();
				endChanged = TRUE;
				lastSlot = end + 1;
				doneEffort = 0.0;
				doneDuration = 0.0;
				doneLength = 0.0;
				planCosts = 0.0;
				workStarted = FALSE;
				tentativeEnd = date + slotDuration - 1;
			}
			else
				return TRUE;	// Task cannot be scheduled yet.
		}
		else if (lastSlot == 0)
			lastSlot = end + 1;
		/* Do not schedule anything if the currently time slot is not
		 * directly preceeding the previously scheduled time slot. */
		if (date + slotDuration != lastSlot)
			return !endChanged;
		lastSlot = date;
	}

	if (length > 0.0 || duration > 0.0)
	{
		/* Length specifies the number of working days (as daily load)
		 * and duration specifies the number of calender days. */
		if (!allocations.isEmpty() && !project->isVacation(date))
			bookResources(date, slotDuration);

		doneDuration += (double) slotDuration / ONEDAY;
		if (!(isWeekend(date) || project->isVacation(date)))
		{
			doneLength += (double) slotDuration / ONEDAY;
			/* Move the start date to make sure that there is
			 * some work going on on the start date. */
			if (!workStarted && allocations.isEmpty())
			{
				if (scheduling == ASAP)
					start = date;
				else if (scheduling == ALAP)
					end = date + slotDuration - 1;
				else
					qFatal("Unknown scheduling mode");
				workStarted = TRUE;
			}
		}

		if ((length > 0.0 && doneLength >= length) ||
			(duration > 0.0 && doneDuration >= duration))
		{
			if (scheduling == ASAP)
				end = tentativeEnd;
			else
				start = tentativeStart;
			return FALSE;
		}
	}
	else if (effort > 0.0)
	{
		if (project->isVacation(date))
			return TRUE;
		/* The effort of the task has been specified. We have to look
		 * how much the resources can contribute over the following
		 * workings days until we have reached the specified
		 * effort. */
		if (allocations.count() == 0)
		{
			fatalError("No allocations specified for effort based task");
			return TRUE;
		}
		bookResources(date, slotDuration);
		if (doneEffort >= effort)
		{
			if (scheduling == ASAP)
				end = tentativeEnd;
			else
				start = tentativeStart;
			return FALSE;
		}
	}
	else
	{
		// Task is a milestone.
		end = start;
		return FALSE;
	}
	return TRUE;
}

bool
Task::scheduleContainer()
{
	Task* t;
	time_t nstart = 0;
	time_t nend = 0;

	// Check that this is really a container task
	if ((t = subTasks.first()))
	{
		/* Make sure that all sub tasks have been scheduled. If not we
		 * can't yet schedule this task. */
		if (t->getStart() == 0 || t->getEnd() == 0)
			return TRUE;
		nstart = t->getStart();
		nend = t->getEnd();
	}
	else
		return TRUE;

	for (t = subTasks.next() ; t != 0; t = subTasks.next())
	{
		/* Make sure that all sub tasks have been scheduled. If not we
		 * can't yet schedule this task. */
		if (t->getStart() == 0 || t->getEnd() == 0)
			return TRUE;

		if (t->getStart() < nstart)
			nstart = t->getStart();
		if (t->getEnd() > nend)
			nend = t->getEnd();
	}

	start = nstart;
	end = nend;
	return FALSE;
}

bool
Task::bookResources(time_t date, time_t slotDuration)
{
	bool allocFound = FALSE;

	for (Allocation* a = allocations.first();
		 a != 0 && (effort == 0.0 || doneEffort < effort);
		 a = allocations.next())
	{
		if (a->isPersistent() && a->getLockedResource())
		{	
			bookResource(a->getLockedResource(), date, slotDuration);
		}
		else if (bookResource(a->getResource(), date, slotDuration))
		{
			allocFound = TRUE;
			if (a->isPersistent())
				a->setLockedResource(a->getResource());
		}
		else
		{
			for (Resource* r = a->first(); r != 0; r = a->next())
				if (bookResource(r, date, slotDuration))
				{
					allocFound = TRUE;
					if (a->isPersistent())
						a->setLockedResource(r);
					break;
				}
		}
	}

	return allocFound;
}

bool
Task::bookResource(Resource* r, time_t date, time_t slotDuration)
{
	Interval interval;

	bool booked = FALSE;
	for (Resource* rit = r->subResourcesFirst(); rit != 0;
		 rit = r->subResourcesNext())
	{
		if ((*rit).isAvailable(date, slotDuration, interval))
		{
			double intervalLoad = project->convertToDailyLoad(
				interval.getDuration());
			(*rit).book(new Booking(interval, this,
								account ? account->getKotrusId() : QString(""),
								project->getId()));
			addBookedResource(rit);

			/* Move the start date to make sure that there is
			 * some work going on on the start date. */
			if (!workStarted)
			{
				if (scheduling == ASAP)
					start = date;
				else if (scheduling == ALAP)
					end = date + slotDuration - 1;
				else
					qFatal("Unknown scheduling mode");
				workStarted = TRUE;
			}

			tentativeStart = interval.getStart();
			tentativeEnd = interval.getEnd();
			doneEffort += intervalLoad * (*rit).getEfficiency();
			planCosts += (*rit).getRate() * intervalLoad *
				(*rit).getEfficiency();

			booked = TRUE;
		}
	}
	return booked;
}

bool
Task::isScheduled()
{
	return ((start != 0 && end != 0) || !subTasks.isEmpty());
}

bool
Task::needsEarlierTimeSlot(time_t date)
{
	if (scheduling == ALAP && end != 0 && start == 0 && date > lastSlot)
		return TRUE;
	if (scheduling == ASAP && start != 0 && end == 0 && date > lastSlot + 1)
		return TRUE;

	return FALSE;
}

bool
Task::isDayCompleted(time_t date) const
{
	// If task is not scheduled for this day, the day can not be completed.
	if ((date < midnight(start)) || (sameTimeNextDay(midnight(end)) <= date))
		return FALSE;

	if (complete != -1)
	{
		// some completion degree was specified.
		return ((complete / 100.0) * (end - start) + start) > midnight(date);
	}
	

	return (project->getNow() > sameTimeNextDay(midnight(date)));
}

time_t
Task::earliestStart()
{
	time_t date = 0;
	for (Task* t = depends.first(); t != 0; t = depends.next())
	{
		// All tasks this task depends on must have an end date set.
		if (t->getEnd() == 0)
			return 0;
		// Milestones are assumed to have duration 0.
		if (t->getEnd() > date)
			date = t->getEnd() + (t->getStart() == t->getEnd() ? 0 : 1);
	}

	return date;
}

time_t
Task::latestEnd()
{
	time_t date = 0;
	for (Task* t = preceeds.first(); t != 0; t = preceeds.next())
	{
		// All tasks this task preceeds must have an start date set.
		if (t->getStart() == 0)
			return 0;
		if (date == 0 || t->getStart() < date)
			date = t->getStart() - 1;
	}

	return date;
}

double
Task::getLoadOnDay(time_t date)
{
	double load = 0.0;

	if (subTasks.first())
	{
		for (Task* t = subTasks.first(); t != 0; t = subTasks.next())
			load += t->getLoadOnDay(date);
		return load;
	}

	for (Resource* r = bookedResources.first(); r != 0;
		 r = bookedResources.next())
	{
		load += r->getLoadOnDay(date, this);
	}
	return load;
}

double
Task::getLoadOnMonth(time_t date)
{
	Interval month(beginOfMonth(date),
				   sameTimeNextMonth(beginOfMonth(date)) - 1);
	return getLoad(month);
}

double
Task::getLoad(const Interval& period)
{
	double load = 0.0;

	if (subTasks.first())
	{
		for (Task* t = subTasks.first(); t != 0; t = subTasks.next())
			load += t->getLoad(period);
		return load;
	}

	for (Resource* r = bookedResources.first(); r != 0;
		 r = bookedResources.next())
	{
		load += r->getLoad(period, this);
	}
	return load;
}

double
Task::getPlanCosts()
{
	double costs = 0.0;

	if (subTasks.first())
	{
		for (Task* t = subTasks.first(); t != 0; t = subTasks.next())
			costs += t->getPlanCosts();
		return costs;
	}

	return planCosts;
}

bool
Task::xRef(QDict<Task>& hash)
{
	bool error = FALSE;

	for (QStringList::Iterator it = dependsIds.begin();
		 it != dependsIds.end(); ++it)
	{
		QString absId = resolveId(*it);
		Task* t;
		if ((t = hash.find(absId)) == 0)
		{
			fatalError(QString("Unknown dependency '") + absId + "'");
			error = TRUE;
		}
		else if (depends.find(t) != -1)
		{
			fatalError(QString("No need to specify dependency '") + absId +
							   "' twice.");
			error = TRUE;
		}
		else
		{
			depends.append(t);
			previous.append(t);
			t->followers.append(this);
		}
	}

	for (QStringList::Iterator it = preceedsIds.begin();
		 it != preceedsIds.end(); ++it)
	{
		QString absId = resolveId(*it);
		Task* t;
		if ((t = hash.find(absId)) == 0)
		{
			fatalError(QString("Unknown dependency '") + absId + "'");
			error = TRUE;
		}
		else if (preceeds.find(t) != -1)
		{
			fatalError(QString("No need to specify dependency '") + absId +
							   "' twice.");
			error = TRUE;
		}
		else
		{
			preceeds.append(t);
			followers.append(t);
			t->previous.append(this);
		}
	}

	return error;
}

QString
Task::resolveId(QString relId)
{
	/* Converts a relative ID to an absolute ID. Relative IDs start
	 * with a number of bangs. A set of bangs means 'Name of the n-th
	 * parent task' with n being the number of bangs. */
	if (relId[0] != '!')
		return relId;

	Task* t = this;
	unsigned int i;
	for (i = 0; i < relId.length() && relId.mid(i, 1) == "!"; ++i)
	{
		if (!t->parent)
		{
			fatalError(QString("Illegal relative ID '") + relId + "'");
			return relId;
		}
		t = t->parent;
	}
	if (t)
		return t->id + "." + relId.right(relId.length() - i);
	else
		return relId.right(relId.length() - i);
}

bool
Task::scheduleOK()
{
	if (!subTasks.isEmpty())
		return TRUE;

	if (start == 0)
	{
		fatalError(QString("Task '") + id + "' has no start time.");
		return false;
	}
	if (end == 0)
	{
		fatalError(QString("Task '") + id + "' has no end time.");
		return false;
	}
	// Check if all previous tasks end before start of this task.
	for (Task* t = previous.first(); t != 0; t = previous.next())
		if (t->getEnd() > start)
		{
			fatalError(QString("Task '") + id + "' cannot follow task '" +
					   t->getId() + "'.");
			return false;
		}
	// Check if all following task start after this tasks end.
	for (Task* t = followers.first(); t != 0; t = followers.next())
		if (end > t->getStart())
		{
			fatalError(QString("Task '") + id + "' cannot preceed task '" +
					   t->getId() + "'.");
			return false;
		}

	return TRUE;
}

bool
Task::isActiveToday(time_t date) const
{
	Interval day(midnight(date), sameTimeNextDay(midnight(date)) - 1);
	Interval work;
	if (isMilestone())
		work = Interval(start, start + 1);
	else
		work = Interval(start, end);
	return day.overlap(work);
}

bool
Task::isActiveThisWeek(time_t date) const
{
	Interval week(beginOfWeek(date), sameTimeNextWeek(midnight(date)) - 1);
	Interval work;
	if (isMilestone())
		work = Interval(start, start + 1);
	else
		work = Interval(start, end);
	return week.overlap(work);
}

bool
Task::isActiveThisMonth(time_t date) const
{
	Interval month(beginOfMonth(date),
				   sameTimeNextMonth(beginOfMonth(date)) - 1);
	Interval work;
	if (isMilestone())
		work = Interval(start, start + 1);
	else
		work = Interval(start, end);
	return month.overlap(work);
}

void
Task::getSubTaskList(TaskList& tl)
{
	for (Task* t = subTasks.first(); t != 0; t = subTasks.next())
	{
		tl.append(t);
		t->getSubTaskList(tl);
	}
}

void
Task::treeSortKey(QString& key)
{
	if (!parent)
		return;
	int i = 1;
	for (Task* t = parent->subTasks.first(); t != 0;
		 t = parent->subTasks.next(), i++)
		if (t == this)
		{
			key = QString().sprintf("%04d", i) + key;
			break;
		}
	parent->treeSortKey(key);
}

QDomElement Task::xmlElement( QDomDocument& doc ) const
{
   QDomElement elem = doc.createElement( "Task" );
   QDomText t;
   
   elem.appendChild( ReportXML::createXMLElem( doc, "TaskName", getName()));
   elem.appendChild( ReportXML::createXMLElem( doc, "Priority", QString::number( priority )));
   elem.appendChild( ReportXML::createXMLElem( doc, "start", QString::number( start )));
   elem.appendChild( ReportXML::createXMLElem( doc, "end", QString::number( end )));
   elem.appendChild( ReportXML::createXMLElem( doc, "minStart", QString::number( minStart )));
   elem.appendChild( ReportXML::createXMLElem( doc, "maxStart", QString::number( maxStart )));
   elem.appendChild( ReportXML::createXMLElem( doc, "minEnd", QString::number( maxStart )));
   elem.appendChild( ReportXML::createXMLElem( doc, "maxEnd", QString::number( maxStart )));
   elem.appendChild( ReportXML::createXMLElem( doc, "actualStart", QString::number( maxStart )));
   elem.appendChild( ReportXML::createXMLElem( doc, "actualEnd", QString::number( maxStart )));

   /* Now start the subtasks */
   int cnt = 0;
   QDomElement subtElem = doc.createElement( "SubTasks" );
   TaskList tl ( subTasks );
   for (Task* t = tl.first(); t != 0; t = tl.next())
   {
      if( t != this )
      {
	 QDomElement sTask = t->xmlElement( doc );
	 subtElem.appendChild( sTask );
	 cnt++;
      }
   }
   if( cnt > 0 )
      elem.appendChild( subtElem );

   /* Tasks (by id) on which this task depends */
   if( dependsIds.count() > 0 )
   {
      QDomElement deps = doc.createElement( "Depends" );
      
      for (QValueListConstIterator<QString> it1= dependsIds.begin(); it1 != dependsIds.end(); ++it1)
      {
	 deps.appendChild( ReportXML::createXMLElem( doc, "TaskID", *it1 ));
      }
      elem.appendChild( deps );
   }

   /* list of tasks by id which are previous */
   if( previous.count() > 0 )
   {
      QDomElement prevs = doc.createElement( "Previous" );

      TaskList tl( previous );
      for (Task* t = tl.first(); t != 0; t = tl.next())
      {	
	 if( t != this )
	 {
	    prevs.appendChild( ReportXML::createXMLElem( doc, "TaskID", t->getId()));
	 }
      }
      elem.appendChild( prevs );
   }
   
   /* list of tasks by id which follow */
   if( followers.count() > 0 )
   {
      QDomElement foll = doc.createElement( "Followers" );

      TaskList tl( followers );
      for (Task* t = tl.first(); t != 0; t = tl.next())
      {	
	 if( t != this )
	 {
	    foll.appendChild( ReportXML::createXMLElem( doc, "TaskID", t->getId()));
	 }
      }

      elem.appendChild( foll );
   }

   /* Allocations */
   if( allocations.count() > 0 )
   {
      QDomElement alloc = doc.createElement( "Allocations" );

      QPtrList<Allocation> al(allocations);
      for (Allocation* a = al.first(); a != 0; a = al.next())
      {
	 alloc.appendChild( a->xmlElement( doc ));
      }
      elem.appendChild( alloc );
   }

   /* booked Ressources */
   if( bookedResources.count() > 0 )
   {	
      QDomElement bres = doc.createElement( "bookedResources" );

      QPtrList<Resource> br(bookedResources);
      for (Resource* r = br.first(); r != 0; r = br.next())
      {
	 bres.appendChild( r->xmlIDElement( doc ));
      }
      elem.appendChild( bres );
   }

   
   /* Comment */
   if( ! note.isEmpty())
   {
      QDomElement e = doc.createElement( "note" );
      elem.appendChild( e );
      t = doc.createTextNode( note );
      e.appendChild( t );
   }

   return( elem );
}

TaskList::TaskList()
{
	sorting = Pointer;
}

TaskList::~TaskList()
{
}

int
TaskList::compareItems(QCollection::Item i1, QCollection::Item i2)
{
	Task* t1 = static_cast<Task*>(i1);
	Task* t2 = static_cast<Task*>(i2);

	switch (sorting)
	{
	case Pointer:
		return t1 == t2 ? 0 : t1 < t2 ? -1 : 1;
	case TaskTree:
	{
		QString key1;
		t1->treeSortKey(key1);
		key1 += QString("0000") + time2ISO(t1->getStart());
		QString key2;
		t2->treeSortKey(key2);
		key2 += QString("0000") + time2ISO(t2->getStart());
		if (key1 == key2)
		{
			/* If the keys are identical we do an inverse sort for the
			 * end date. That way the parent tasks are sorted above
			 * their childs. */
			return t1->getEnd() == t2->getEnd() ? 0 :
				t1->getEnd() > t2->getEnd() ? -1 : 1;
		}
		else
			return key1 < key2 ? -1 : 1;
	}
	case StartUp:
		return t1->getStart() == t2->getStart() ? 0 :
			t1->getStart() > t2->getStart() ? -1 : 1;
	case StartDown:
		return t1->getStart() == t2->getStart() ? 0 :
			t1->getStart() < t2->getStart() ? -1 : 1;
	case EndUp:
		return t1->getEnd() == t2->getEnd() ? 0 :
			t1->getEnd() > t2->getEnd() ? -1 : 1;
	case EndDown:
		return t1->getEnd() == t2->getEnd() ? 0 :
			t1->getEnd() < t2->getEnd() ? -1 : 1;
	case PrioUp:
		if (t1->getPriority() == t2->getPriority())
			return 0; // TODO: Use duration as next criteria
		else
			return (t1->getPriority() - t2->getPriority());
	case PrioDown:
		if (t1->getPriority() == t2->getPriority())
			return 0; // TODO: Use duration as next criteria
		else
			return (t2->getPriority() - t1->getPriority());
	default:
		qFatal("Unknown sorting criteria!\n");
		return 0;
	}		
}
