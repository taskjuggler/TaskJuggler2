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
Account::getPlanVolume(const Interval& period)
{
	TaskList tl = project->getTaskList();

	double volume = 0.0;
	// Add plan credits for all tasks that should be credited to this account.
	for (Task* t = tl.first(); t != 0; t = tl.next())
		if (t->getAccount() == this)
			volume += t->getPlanCredits(period, 0, FALSE);
	// Add all transactions that are registered within the period.
	for (Transaction* t = transactions.first(); t != 0;
		 t = transactions.next())
		if (period.contains(t->getDate()))
			volume += t->getAmount();

	return volume;
}

double
Account::getActualVolume(const Interval& period)
{
	TaskList tl = project->getTaskList();

	double volume = 0.0;
	/* Add actual credits for all tasks that should be credited to this
	 * account. */
	for (Task* t = tl.first(); t != 0; t = tl.next())
		if (t->getAccount() == this)
			volume += t->getActualCredits(period, 0, FALSE);
	// Add all transactions that are registered within the period.
	for (Transaction* t = transactions.first(); t != 0;
		 t = transactions.next())
		if (period.contains(t->getDate()))
			volume += t->getAmount();

	return volume;
}

double
Account::getPlanBalance(time_t /* date */)
{
	return 0.0;
}

double
Account::getActualBalance(time_t /* date */)
{
	return 0.0;
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
		QString fn1;
		// c1->getFullName(fn1);
		fn1 = (a1->getType() == Account::Cost ? QString("0") : QString("1"))
			+ fn1;
		QString fn2;
		// c2->getFullName(fn2);
		fn2 = (a2->getType() == Account::Cost ? QString("0") : QString("1"))
			+ fn2;
		return fn1.compare(fn2);
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
