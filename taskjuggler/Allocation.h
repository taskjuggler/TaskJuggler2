/*
 * Allocation.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

class Allocation
{
public:
	Allocation(Resource* r);

	~Allocation() { }

	void setLoad(int l) { load = l; }
	int getLoad() const { return load; }

	void setPersistent(bool p) { persistent = p; }
	bool isPersistent() const { return persistent; }

	void setLockedResource(Resource* r) { lockedResource = r; }
	Resource* getLockedResource() const { return lockedResource; }

	void addCandidate(Resource* r) { candidates.append(r); }
	QPtrList<Resource> getCandidates() const { return candidates; }
	Resource* first() { return candidates.first(); }
	Resource* next() { return candidates.next(); }

	bool addShift(const Interval& i, Shift* s)
	{
		return shifts.insert(new ShiftSelection(i, s));
	}

	bool isOnShift(const Interval& i)
	{
		return shifts.count() == 0 || shifts.isOnShift(i);
	}

	enum SelectionModeType { order, minLoaded, maxLoaded, random };
	void setSelectionMode(SelectionModeType smt) { selectionMode = smt; }
	bool setSelectionMode(const QString& smt);
	SelectionModeType getSelectionMode() const { return selectionMode; }

	QDomElement xmlElement(QDomDocument& doc);

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

   /// The persintent resource picked by the scheduler.
   Resource* lockedResource;
   
   /// List of potential resources.
   QPtrList<Resource> candidates;

   /* The selection mode determines how the resource is selected from
    * the candidate list. */
   SelectionModeType selectionMode;
} ;

