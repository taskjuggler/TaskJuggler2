/*
 * Allocation.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef _Allocation_h_
#define _Allocation_h_

#include <qdom.h>

#include "ShiftSelectionList.h"
#include "ShiftSelection.h"

class Resource;

class Allocation
{
public:
    Allocation(Resource* r);
    Allocation(const Allocation& a);

    ~Allocation() { }

    void setLoad(int l) { load = l; }
    int getLoad() const { return load; }

    void setPersistent(bool p) { persistent = p; }
    bool isPersistent() const { return persistent; }

    void setMandatory(bool m) { mandatory = m; }
    bool isMandatory() const { return mandatory; }

    void setLockedResource(Resource* r) { lockedResource = r; }
    Resource* getLockedResource() const { return lockedResource; }

    void addCandidate(Resource* r) { candidates.append(r); }
    QPtrListIterator<Resource> getCandidatesIterator() const
    {
        return QPtrListIterator<Resource>(candidates);
    }
    QPtrList<Resource> getCandidates() const { return candidates; }

    bool addShift(const Interval& i, Shift* s)
    {
        return shifts.insert(new ShiftSelection(i, s));
    }

    bool isOnShift(const Interval& i)
    {
        return shifts.isOnShift(i);
    }

    enum SelectionModeType { order, minLoaded, maxLoaded, random };
    void setSelectionMode(SelectionModeType smt) { selectionMode = smt; }
    bool setSelectionMode(const QString& smt);
    SelectionModeType getSelectionMode() const { return selectionMode; }

    QDomElement xmlElement(QDomDocument& doc);

    void setConflictStart(time_t cs) { conflictStart = cs; }
    time_t getConflictStart() const { return conflictStart; }

    void init()
    {
        lockedResource = 0;
        conflictStart = 0;
    }
    
private:
    /// Don't use this.
    Allocation();

    /// The maximum daily usage of the resource in percent.
    int load;

    /// The shifts that can limit the allocation to certain intervals.
    ShiftSelectionList shifts;

    /**
     * True if the allocation should be persistent over the whole task.
     * If set the first selection will not be changed even if there is an
     * available alternative. */
    bool persistent;

    /**
     * True if the resource is mandatory. This means, that only if this
     * allocation can be made, all other allocations will be made. If this
     * allocation cannot be made for a time slot, no other allocations will be
     * made for the task.
     */
    bool mandatory;

    /// The persintent resource picked by the scheduler.
    Resource* lockedResource;

    /// Start of a resource conflict. Used during scheduling only.
    time_t conflictStart;

    /// List of potential resources.
    QPtrList<Resource> candidates;

    /* The selection mode determines how the resource is selected from
     * the candidate list. */
    SelectionModeType selectionMode;
} ;

#endif
