/*
 * task.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
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

#include "taskjuggler.h"
#include "debug.h"
#include "TaskList.h"
#include "TaskScenario.h"
#include "ResourceList.h"
#include "CoreAttributes.h"
#include "ShiftSelectionList.h"
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
class Shift;
class TaskList;
class QDomElement;
class QDomDocument;
class Allocation;
class Interval;

/**
 * This class stores all task related information and provides methods to
 * manipulte them. It provides fundamental functions like the scheduler.
 *
 * @short The class that holds all task related information.
 * @see Resource
 * @see CoreAttributes
 * @author Chris Schlaeger <cs@suse.de>
 */
class Task : public CoreAttributes
{
    friend int TaskList::compareItemsLevel(Task*, Task*, int);

public:
    Task(Project* prj, const QString& id_, const QString& n, Task* p,
         const QString& f, int l);
    virtual ~Task();

    virtual const char* getType() const { return "Task"; }

    Task* getParent() const { return (Task*) parent; }

    TaskListIterator getSubListIterator() const
    {
        return TaskListIterator(sub);
    }

    enum SchedulingInfo { ASAP, ALAP };

    enum Scenario { Plan = 0, Actual };

    void setProjectId(const QString& i) { projectId = i; }
    const QString& getProjectId() const { return projectId; }

    void setNote(const QString& d) { note = d; }
    const QString& getNote() const { return note; }

    void setReference(const QString& r, const QString& l)
    {
        ref = r;
        refLabel = l;
    }
    const QString& getReference() const { return ref; }
    const QString& getReferenceLabel() const { return refLabel; }

    void setScheduling(SchedulingInfo si) { scheduling = si; }
    SchedulingInfo getScheduling() const { return scheduling; }

    void setPriority(int p) { priority = p; }
    int getPriority() const { return priority; }

    void setResponsible(Resource* r) { responsible = r; }
    Resource* getResponsible() const { return responsible; }

    void setMilestone() { milestone = TRUE; }
    bool isMilestone() const { return milestone; }

    void setAccount(Account* a) { account = a; }
    Account* getAccount() const { return account; }

    bool addDepends(const QString& id);
    bool addPrecedes(const QString& id);

    bool addShift(const Interval& i, Shift* s);

    void addAllocation(Allocation* a) { allocations.append(a); }
    QPtrListIterator<Allocation> getAllocationIterator() const
    {
        return QPtrListIterator<Allocation>(allocations);
    }

    TaskListIterator getPreviousIterator() const
    {
        return TaskListIterator(previous);
    }
    bool hasPrevious() const { return !previous.isEmpty(); }

    TaskListIterator getFollowersIterator() const
    {
        return TaskListIterator(followers);
    }
    bool hasFollowers() const { return !followers.isEmpty(); }

    bool hasPrevious(Task* t) { return previous.findRef(t) != -1; }
    bool hasFollower(Task* t) { return followers.findRef(t) != -1; }

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

    void setMinStart(int sc, time_t s) { scenarios[sc].minStart = s; }
    time_t getMinStart(int sc) const { return scenarios[sc].minStart; }

    void setMaxStart(int sc, time_t s) { scenarios[sc].maxStart = s; }
    time_t getMaxStart(int sc) const { return scenarios[sc].maxStart; }

    void setMinEnd(int sc, time_t e) { scenarios[sc].minEnd = e; }
    time_t getMinEnd(int sc) const { return scenarios[sc].minEnd; }

    void setMaxEnd(int sc, time_t e) { scenarios[sc].maxEnd = e; }
    time_t getMaxEnd(int sc) const { return scenarios[sc].maxEnd; }

    void setLength(int sc, double days) { scenarios[sc].length = days; }
    double getLength(int sc) const { return scenarios[sc].length; }

    void setEffort(int sc, double e) { scenarios[sc].effort = e; }
    double getEffort(int sc) const { return scenarios[sc].effort; }

    void setDuration(int sc, double d) { scenarios[sc].duration = d; }
    double getPlanDuration(int sc) const { return scenarios[sc].duration; }

    bool isStartOk(int sc) const
    {
        return scenarios[sc].isStartOk();
    }
    bool isEndOk(int sc) const
    {
        return scenarios[sc].isEndOk(milestone);
    }

    bool isBuffer(int sc, const Interval& iv) const;

    void setComplete(int sc, int c) { scenarios[sc].complete = c; }
    double getComplete(int sc) const { return scenarios[sc].complete; }

    void setStatusNote(int sc, const QString& d)
    {
        scenarios[sc].statusNote = d;
    }
    const QString& getStatusNote(int sc) const
    {
        return scenarios[sc].statusNote;
    }

    void setStartBuffer(int sc, double p) { scenarios[sc].startBuffer = p; }
    double getStartBuffer(int sc) const { return scenarios[sc].startBuffer; }

    void setEndBuffer(int sc, double p) { scenarios[sc]. endBuffer = p; }
    double getEndBuffer(int sc) const { return scenarios[sc].endBuffer; }

    void setStartCredit(int sc, double c) { scenarios[sc].startCredit = c; }
    double getStartCredit(int sc) const { return scenarios[sc].startCredit; }

    void setEndCredit(int sc, double c) { scenarios[sc].endCredit = c; }
    double getEndCredit(int sc) const { return scenarios[sc].endCredit; }

    double getCalcEffort(int sc) const;
    double getCalcDuration(int sc) const;

    double getCredits(int sc, const Interval& period, AccountType acctType,
                      const Resource* resource = 0, bool recursive = TRUE)
        const;

    bool isActive(int sc, const Interval& period) const;
    TaskStatus getStatus(int sc) const { return scenarios[sc].status; }
    bool isCompleted(int sc, time_t date) const;
    void calcCompletionDegree(int sc);
    double getCompletionDegree(int sc) const;
    double getCalcedCompletionDegree(int sc) const;
    TaskStatus getCompletionStatus(int sc) const
    {
        return scenarios[sc].status;
    }

    double getLoad(int sc, const Interval& period, const Resource* resource = 0)
        const;

    void addBookedResource(int sc, Resource* r)
    {
        if (scenarios[sc].bookedResources.find((CoreAttributes*) r) == -1)
            scenarios[sc].bookedResources.inSort((CoreAttributes*) r);
    }
    bool isBookedResource(int sc, Resource* r)
    {
        return scenarios[sc].bookedResources.find((CoreAttributes*) r) != -1;
    }
    ResourceListIterator getBookedResourcesIterator(int sc) const
    {
        return ResourceListIterator(scenarios[sc].bookedResources);
    }
    ResourceList getBookedResources(int sc) const
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

    bool preScheduleOk();
    bool scheduleOk(int sc, int& errors) const;
    void initLoopDetector()
    {
        loopDetectorMarkStart = FALSE;
        loopDetectorMarkEnd = FALSE;
    }
    bool loopDetector();
    void computeBuffers();
    time_t nextSlot(time_t slotDuration) const;
    void schedule(time_t& reqStart, time_t duration);
    void propagateStart(bool safeMode = TRUE);
    void propagateEnd(bool safeMode = TRUE);
    void propagateInitialValues();
    void setRunaway();
    bool isRunaway() const;

    bool isSubTask(Task* t) const;

    void errorMessage(const char*, ...) const;

    QDomElement xmlElement( QDomDocument& doc, bool absId = true );

#ifdef HAVE_ICAL
#ifdef HAVE_KDE
   void toTodo( KCal::Todo *, KCal::CalendarLocal * );
#endif
#endif
   void loadFromXML( QDomElement& parent, Project *p );
    void allocationFromXML( const QDomElement& );
private:
    bool loopDetection(LDIList& list, bool atEnd, LoopDetectorInfo::FromWhere
                       caller);
    bool scheduleContainer(bool safeMode);
    Task* subFirst() { return (Task*) sub.first(); }
    Task* subNext() { return (Task*) sub.next(); }
    bool bookResource(Resource* r, time_t day, time_t duration, int loadFactor);
    bool bookResources(time_t day, time_t duration);
    void addBookedResource(Resource* r)
    {
        if (bookedResources.find((CoreAttributes*) r) == -1)
            bookedResources.inSort((CoreAttributes*) r);
    }
    QPtrList<Resource> createCandidateList(time_t date, Allocation* a);
    time_t earliestStart() const;
    time_t latestEnd() const;

    bool hasStartDependency(int sc);
    bool hasEndDependency(int sc);

    bool hasStartDependency();
    bool hasEndDependency();

    bool dependsOnABrother(const Task* p) const;
    bool precedesABrother(const Task* p) const;

    /// A longer description of the task.
    QString note;

    /// A reference to an external document
    QString ref;

    /// A label used instead of the reference
    QString refLabel;

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
     * A list of tasks that have requested to precede this task.
     */
    TaskList predecessors;

    /**
     * A list of tasks that have requested to depend on this task.
     */
    TaskList successors;

    /**
     * A list of all tasks that preceed this task. It's the combination of
     * depends and predecessors. This is redundant information but stored for
     * conveniance. Interdependent tasks are linked in a doubly linked list.
     */
    TaskList previous;

    /**
     * A list of all tasks that follow this task. It's the combination of
     * precedes and successors. This is redundant information but stored for
     * conveniance. Interdependent tasks are linked in a doubly linked list.
     */
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

    TaskScenario* scenarios;

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

    /**
     * The loop detector marks are used during dependency loop detection only.
     */
    bool loopDetectorMarkStart;
    bool loopDetectorMarkEnd;

    /// A list of all the resources booked for this task.
    ResourceList bookedResources;
} ;

#endif
