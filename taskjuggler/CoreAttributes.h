/*
 * CoreAttributes.h - TaskJuggler
 *
 * Copyright (c) 2001 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms version 2 of the GNU General Public License as
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

class CoreAttributesList : public QPtrList<CoreAttributes>
{
public:
	CoreAttributesList() { sorting = Pointer; }
	virtual ~CoreAttributesList();

	enum SortCriteria { Pointer, TreeMode, NameUp, NameDown, FullNameUp,
						FullNameDown, IdUp, IdDown, IndexUp, IndexDown, 
						StartUp, StartDown, EndUp, EndDown,
						PrioUp, PrioDown,
						ResponsibleUp, ResponsibleDown };

	void setSorting(SortCriteria s);
	void createIndex();

protected:
	virtual int compareItems(QCollection::Item i1, QCollection::Item i2);

	SortCriteria sorting;
} ;

class CoreAttributes
{
public:
	CoreAttributes(Project* p, const QString& i, const QString& n,
				   CoreAttributes* parent_) :
		project(p), id(i), name(n), parent(parent_) { }
	virtual ~CoreAttributes() { }

	const QString& getId() const { return id; }

	void setIndex(uint idx) { index = idx; }
	uint getIndex() const { return index; }

	void setName(const QString& n) { name = n; }
	const QString& getName() const { return name; }
	void getFullName(QString& fullName);

	CoreAttributes* getParent() const { return parent; }

	void addSub(CoreAttributes* c) { sub.append(c); }
	virtual CoreAttributesList getSubList()  const { return sub; }

	void addFlag(QString flag) { flags.addFlag(flag); }
	void clearFlag(const QString& flag) { flags.clearFlag(flag); }
	bool hasFlag(const QString& flag) { return flags.hasFlag(flag); }
	FlagList getFlagList() const { return flags; }

protected:
	/// A pointer to access information that are global to the project.
	Project* project;

	/// An ID that must be unique within the attribute class.
	QString id;

	/// An index number that must be unique within the attribute class.
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

#endif
