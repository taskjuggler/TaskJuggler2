/*
 * Account.h - TaskJuggler
 *
 * Copyright (c) 2001 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "Account.h"

int
TransactionList::compareItems(QCollection::Item i1, QCollection::Item i2)
{
	Transaction* t1 = static_cast<Transaction*>(i1);
	Transaction* t2 = static_cast<Transaction*>(i2);

	if (t1->date == t2->date)
		return 0;
	else
		return t2->date - t1->date;
}

int
AccountList::compareItems(QCollection::Item i1, QCollection::Item i2)
{
	Account* a1 = static_cast<Account*>(i1);
	Account* a2 = static_cast<Account*>(i2);

	return a1->getId().compare(a2->getId());
}

