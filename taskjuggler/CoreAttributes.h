/*
 * CoreAttributes.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _CoreAttributes_h_
#define _CoreAttributes_h_

#include <qstring.h>

#include "FlagList.h"

class Project;
class CoreAttributes;

/**
 * @short The class stores a list of CoreAttributes.
 * @see CoreAttributes 
 * @author Chris Schlaeger <cs@suse.de>
 */
class CoreAttributesList : public QPtrList<CoreAttributes>
{
public:
	CoreAttributesList()
   	{
		for (int i = 0; i < maxSortingLevel; i++)
			sorting[i] = SequenceUp;
   	}
	CoreAttributesList(const CoreAttributesList& l) :
		QPtrList<CoreAttributes>(l)
	{
		for (int i = 0; i < maxSortingLevel; i++)
			sorting[i] = l.sorting[i];
	}
	virtual ~CoreAttributesList();

	enum SortCriteria {
	   	SequenceUp = 0, SequenceDown,
		TreeMode, NameUp, NameDown, FullNameUp,
		FullNameDown, IdUp, IdDown, IndexUp, IndexDown, 
		StatusUp, StatusDown, CompletedUp, CompletedDown,
		PrioUp, PrioDown,
		ResponsibleUp, ResponsibleDown,
		MinEffortUp, MinEffortDown,
		MaxEffortUp, MaxEffortDown,
		RateUp, RateDown,
		KotrusIdUp, KotrusIdDown,
		StartUp, StartDown, EndUp, EndDown
	};

	static const int maxSortingLevel = 3;
	void setSorting(int s, int level);
	void createIndex(bool initial = FALSE);
	uint maxDepth();

	static bool isSupportedSortingCriteria(int sc);

	virtual int compareItemsLevel(CoreAttributes* c1, CoreAttributes* c2,
								  int level);
	
protected:
	virtual int compareItems(QCollection::Item i1, QCollection::Item i2);

	int sorting[maxSortingLevel];
} ;

/**
 * @short Iterator for CoreAttributesList objects.
 * @author Chris Schlaeger <cs@suse.de>
 */
class CoreAttributesListIterator : public QPtrListIterator<CoreAttributes>
{
public:
	CoreAttributesListIterator(const CoreAttributesList& l) :
		QPtrListIterator<CoreAttributes>(l) { }
	virtual ~CoreAttributesListIterator() { }
} ;

class CoreAttributesTreeIterator
{
public:
	CoreAttributesTreeIterator(CoreAttributes* root);
	~CoreAttributesTreeIterator() { }

	CoreAttributes* operator*() { return current; }
	CoreAttributes* operator++();

protected:
	CoreAttributes* current;
private:
	CoreAttributes* root;
} ;

/**
 * @short This class is the base class for all attribute classes.
 * @author Chris Schlaeger <cs@suse.de>
 */
class CoreAttributes
{
public:
	CoreAttributes(Project* p, const QString& i, const QString& n,
				   CoreAttributes* parent_) :
		project(p), id(i), name(n), parent(parent_)
	{
		if (parent_)
			parent_->sub.append(this);
   	}
	virtual ~CoreAttributes() { }

	virtual const char* getType() const { return "CoreAttributes"; }

	const QString& getId() const { return id; }
	QString getFullId() const;
	
	void setIndex(uint idx) { index = idx; }
	uint getIndex() const { return index; }

	void setSequenceNo(uint no) { sequenceNo = no; }
	uint getSequenceNo() const { return sequenceNo; }
	
	Project* getProject() const { return project; }

	void setName(const QString& n) { name = n; }
	const QString& getName() const { return name; }
	void getFullName(QString& fullName) const;

	CoreAttributes* getParent() const { return parent; }

	uint treeLevel() const;

	CoreAttributesList getSubList() const { return sub; }
	CoreAttributesListIterator getSubListIterator() const 
	{ 
		return CoreAttributesListIterator(sub);
	}
	bool hasSubs() const { return !sub.isEmpty(); }
	void addFlag(QString flag) { flags.addFlag(flag); }
	void clearFlag(const QString& flag) { flags.clearFlag(flag); }
	bool hasFlag(const QString& flag) { return flags.hasFlag(flag); }
	FlagList getFlagList() const { return flags; }

	bool hasSameAncestor(CoreAttributes* c);

protected:
	/// A pointer to access information that are global to the project.
	Project* project;

	/// An ID that must be unique within the attribute class.
	QString id;

	/**
     * The index of the attribute declaration within the project files. Each
	 * attribute lists has it's own indices.
	 */
	uint sequenceNo;

	/** The index of the attributes in a logical order that takes the tree
	 * structure and the start and end date into account. Each attribute list
	 * has it's own indices.
	 */
	uint index;

	/// A short description of the attribute.
	QString name;

	/// Pointer to parent. If there is no parent the pointer is 0.
	CoreAttributes* parent;

	/// List of child attributes. 
	CoreAttributesList sub;

	/// List of flags set for this attribute.
	FlagList flags;

	CoreAttributes() { }	// Don't use this!
} ;

template<class TL, class T> int compareTreeItemsT(TL* list, T* c1, T* c2)
{
	if (c1 == c2)
		return 0;

	QPtrList<T> cl1, cl2;
	int res1 = 0;
	for ( ; c1 || c2; )
	{
		if (c1)
		{
			cl1.prepend(c1);
			c1 = c1->getParent();
		}
		else
			res1 = -1;
		if (c2)
		{
			cl2.prepend(c2);
			c2 = c2->getParent();
		}
		else
			res1 = 1;
	}

	QPtrListIterator<T> cal1(cl1);
	QPtrListIterator<T> cal2(cl2); 
	for ( ; *cal1 != 0 && *cal2 != 0; ++cal1, ++cal2)
	{
		int res;
		for (int j = 1; j < CoreAttributesList::maxSortingLevel; ++j)
		{
			if ((res = list->compareItemsLevel(*cal1, *cal2, j)) != 0)
				return res;
		}
		if ((res = (*cal1)->getSequenceNo() - (*cal2)->getSequenceNo()) != 0)
			return res < 0 ? -1 : 1;
	}
	return res1;
}

#endif
