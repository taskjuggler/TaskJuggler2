/*
 * CoreAttributes.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@suse.de>
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
#include <qdict.h>

#include "FlagList.h"
#include "CustomAttribute.h"

class Project;
class CoreAttributes;
class CoreAttributesList;
class CoreAttributesListIterator;
class CustomAttributeDefinition;

/**
 * @short This class is the base class for all attribute classes.
 * @author Chris Schlaeger <cs@suse.de>
 */
class CoreAttributes
{
public:
    CoreAttributes(Project* p, const QString& i, const QString& n,
                   CoreAttributes* parent_);
    virtual ~CoreAttributes();

    virtual CAType getType() const { return CA_Undefined; }

    const QString& getId() const { return id; }
    QString getFullId() const;
    
    void setIndex(int idx) { index = idx; }
    int getIndex() const { return index; }

    void setSequenceNo(uint no) { sequenceNo = no; }
    uint getSequenceNo() const { return sequenceNo; }

    void setHierarchNo(uint no);
    QString getHierarchNo() const;

    void setHierarchIndex(uint no);
    QString getHierarchIndex() const;
    
    Project* getProject() const { return project; }

    void setName(const QString& n) { name = n; }
    const QString& getName() const { return name; }
    void getFullName(QString& fullName) const;

    CoreAttributes* getParent() const { return parent; }

    uint treeLevel() const;

    CoreAttributesList getSubList() const;
    CoreAttributesListIterator getSubListIterator() const; 
    
    bool hasSubs() const;
    void addFlag(QString flag) { flags.addFlag(flag); }
    void clearFlag(const QString& flag) { flags.clearFlag(flag); }
    bool hasFlag(const QString& flag) { return flags.hasFlag(flag); }
    FlagList getFlagList() const { return flags; }

    bool hasSameAncestor(const CoreAttributes* c) const;
    bool isDescendentOf(const CoreAttributes* c) const;
    bool isParentOf(const CoreAttributes* c) const;

    bool isRoot() const { return parent == 0; }
    bool isLeaf() const;

    void addCustomAttribute(const QString& id, CustomAttribute* ca);
    const CustomAttribute* getCustomAttribute(const QString& id) const;
    const QDict<const CustomAttribute>& getCustomAttributeDict() const
    {
        return (const QDict<const CustomAttribute>&) customAttributes;
    }
    void inheritCustomAttributes
        (const QDict<const CustomAttributeDefinition>& dict);

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

    /**
     * The index of the attribute declaration within it's parents childs.
     */
    uint hierarchNo;
    /**
     * The index of the attributes in a logical order that takes the tree
     * structure and the start and end date into account. Each attribute list
     * has it's own indices.
     */
    int index;

    /**
     * The index of the attributes of the same parent in a logical order that
     * takes the tree structure and the start and end date into account. Each
     * attribute list has it's own indices.
     */
    uint hierarchIndex;
    
    /// A short description of the attribute.
    QString name;

    /// Pointer to parent. If there is no parent the pointer is 0.
    CoreAttributes* parent;

    /// List of child attributes. 
    CoreAttributesList* sub;

    /// List of flags set for this attribute.
    FlagList flags;

    /// User defined, optional attributes.
    QDict<CustomAttribute> customAttributes;

    CoreAttributes() { }    // Don't use this!
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
