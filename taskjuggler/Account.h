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

#ifndef _Account_h_
#define _Account_h_

#include <qstring.h>
#include <qlist.h>
#include <time.h>

#include "CoreAttributes.h"

class Task;
class TransactionList;
class Interval;

/**
 * @short Stores all transaction related infromation.
 * @author Chris Schlaeger <cs@suse.de>
 */
class Transaction
{
	friend class TransactionList;
public:
	Transaction(time_t d, double a, const QString& descr)
		: date(d), amount(a), description(descr) { }
	~Transaction() { }

	time_t getDate() { return date; }
	double getAmount() { return amount; }
	const QString& getDescription() { return description; }

private:
	Transaction() { } 	// dont use this
	/// The moment when the transaction happened.
	time_t date;
	/// The amount deposited or withdrawn.
	double amount;
	/// A short description of the transaction purpose
	QString description;
} ;

/**
 * @short A list of transactions.
 * @author Chris Schlaeger <cs@suse.de>
 */
class TransactionList : public QList<Transaction>
{
public:
	TransactionList() { }
	~TransactionList() { }
protected:
	virtual int compareItems(QCollection::Item i1, QCollection::Item i2);
} ;

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

	Account* first() { return (Account*) CoreAttributesList::first(); }
	Account* next() { return (Account*) CoreAttributesList::next(); }

	inline void addAccount(Account* a);
	inline Account* getAccount(const QString& id);

	static bool isSupportedSortingCriteria(int sc);
	
	virtual int compareItemsLevel(Account* a1, Account* a2, int level);

protected:
	virtual int compareItems(QCollection::Item i1, QCollection::Item i2);
} ;

/**
 * @short Stores all account related information.
 * @author Chris Schlaeger <cs@suse.de>
 */
class Account : public CoreAttributes
{
public:
	enum AccountType { Cost, Revenue };

	Account(Project* p, const QString& i, const QString& n, Account* pr,
			AccountType at)
		: CoreAttributes(p, i, n, pr), acctType(at)
	{
		kotrusId = "";
	}
	virtual ~Account() { };

	virtual const char* getType() { return "Account"; }

	Account* getParent() { return (Account*) parent; }

	virtual AccountList getSubList() { return (AccountList&) sub; }

	void setKotrusId(const QString& k) { kotrusId = k; }
	const QString& getKotrusId() const { return kotrusId; }

	void setAcctType(AccountType at) { acctType = at; }
	AccountType getAcctType() const { return acctType; }

	void credit(Transaction* t)
	{
		transactions.inSort(t);
	}

	bool isGroup() const { return !sub.isEmpty(); }

	double getBalance(int sc, time_t d);
	double getVolume(int sc, const Interval& period);

private:
	Account() { };	// don't use this
	QString kotrusId;
	QList<Transaction> transactions;
	AccountType acctType;
} ;

void
AccountList::addAccount(Account* a)
{
	append(a);
}

Account*
AccountList::getAccount(const QString& id)
{
	for (Account* a = first(); a != 0; a = next())
		if (a->getId() == id)
			return a;
	return 0;
}

#endif
