/*
 * Account.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "Account.h"
#include "Interval.h"
#include "Task.h"
#include "Project.h"

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

double
Account::getVolume(int sc, const Interval& period)
{
	TaskList tl = project->getTaskList();

	double volume = 0.0;
	// Add plan credits for all tasks that should be credited to this account.
	for (Task* t = tl.first(); t != 0; t = tl.next())
		if (t->getAccount() == this)
			volume += t->getCredits(sc, period, 0, FALSE);
	// Add all transactions that are registered within the period.
	for (Transaction* t = transactions.first(); t != 0;
		 t = transactions.next())
		if (period.contains(t->getDate()))
			volume += t->getAmount();

	return volume;
}

double
Account::getBalance(int /*sc*/, time_t /* date */)
{
	return 0.0;
}

bool
AccountList::isSupportedSortingCriteria(int sc)
{
	switch (sc)
	{
	case TreeMode:
		return TRUE;
	default:
		return CoreAttributesList::isSupportedSortingCriteria(sc);
	}		
}

int
AccountList::compareItemsLevel(Account* a1, Account* a2, int level)
{
	if (level > 2)
		return -1;

	switch (sorting[level])
	{
	case TreeMode:
	{
		/* Since we like to show all cost accounts first, we have add a
		 * bit of extra code to the usual tree sort handling. */
		if (a1->getType() == Account::Cost &&
			a2->getType() != Account::Cost)
			return -1;
		if (a1->getType() != Account::Cost &&
			a2->getType() == Account::Cost)
			return 1;
		if (level == 0)
			return compareTreeItemsT(this, a1, a2);
		else
			return a1->getSequenceNo() == a2->getSequenceNo() ? 0 :
				a1->getSequenceNo() < a2->getSequenceNo() ? -1 : 1;
	}
	default:
		return CoreAttributesList::compareItemsLevel(a1, a2, level);
	}		
}

int
AccountList::compareItems(QCollection::Item i1, QCollection::Item i2)
{
	Account* a1 = static_cast<Account*>(i1);
	Account* a2 = static_cast<Account*>(i2);

	int res;
	for (int i = 0; i < CoreAttributesList::maxSortingLevel; ++i)
		if ((res = compareItemsLevel(a1, a2, i)) != 0)
			return res;
	return res;
}
