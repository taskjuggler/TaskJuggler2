/*
 * Account.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
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
Account::getVolume(int sc, const Interval& period) const
{
	TaskList tl = project->getTaskList();

	double volume = 0.0;
	// Add plan credits for all tasks that should be credited to this account.
	for (TaskListIterator tli(tl); *tli != 0; ++tli)
		if ((*tli)->getAccount() == this)
			volume += (*tli)->getCredits(sc, period, 0, FALSE);
	// Add all transactions that are registered within the period.
	for (TransactionListIterator tli(transactions); *tli != 0; ++tli)
		if (period.contains((*tli)->getDate()))
			volume += (*tli)->getAmount();

	return volume;
}

double
Account::getBalance(int /*sc*/, time_t /* date */) const
{
	return 0.0;
}

void 
Account::credit(Transaction* t)
{
	transactions.inSort(t);
}
