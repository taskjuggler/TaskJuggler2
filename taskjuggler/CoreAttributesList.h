/*
 * CoreAttributesList.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef _CoreAttributesList_h_
#define _CoreAttributesList_h_

#include <qptrlist.h>

#include "CoreAttributes.h"

class QString;

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
        StartUp, StartDown, EndUp, EndDown,
        CriticalnessUp, CriticalnessDown,
        PathCriticalnessUp, PathCriticalnessDown
    };

    static const int maxSortingLevel = 3;
    void setSorting(int s, int level);
    void createIndex(bool initial = FALSE);
    int getIndex(const QString& id) const;
    uint maxDepth() const;

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

