/*
 * AccountList.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef _AccountList_h_
#define _AccountList_h_

#include "CoreAttributesList.h"

class QString;
class Account;

/**
 * @short A list of accounts.
 * @author Chris Schlaeger <cs@suse.de>
 */
class AccountList : public CoreAttributesList
{
public:
    AccountList() 
    { 
        sorting[0] = CoreAttributesList::TreeMode;
        sorting[1] = CoreAttributesList::IdUp;
    }
    ~AccountList() { }

    Account* getAccount(const QString& id) const;

    static bool isSupportedSortingCriteria(int sc);
    
    virtual int compareItemsLevel(Account* a1, Account* a2, int level);

protected:
    virtual int compareItems(QCollection::Item i1, QCollection::Item i2);
} ;

/**
 * @short Iterator class for AccountList objects.
 * @see AccountList
 * @author Chris Schlaeger <cs@suse.de>
 */
class AccountListIterator : public virtual CoreAttributesListIterator 
{
public:
    AccountListIterator(const CoreAttributesList& l) :
        CoreAttributesListIterator(l) { }
    virtual ~AccountListIterator() { }
    Account* operator*() { return (Account*) get(); }
} ;

#endif

