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

#ifndef _Account_h_
#define _Account_h_

#include <qstring.h>
#include <qlist.h>
#include <time.h>

class Task;
class TransactionList;

class Transaction
{
	friend TransactionList;
public:
	Transaction(time_t d, double a, Task* t = 0)
		: date(d), amount(a) { }
	~Transaction() { }

	time_t getDate() { return date; }
	double getAmount() { return amount; }
	Task* getTask() { return task; }

private:
	Transaction() { } 	// dont use this
	// The date when the tansaction happened.
	time_t date;
	// The amount deposited or withdrawn.
	double amount;
	// The task that caused the transaction (0 if no task).
	Task* task;
} ;

class TransactionList : public QList<Transaction>
{
public:
	TransactionList() { }
	~TransactionList() { }
protected:
	virtual int compareItems(QCollection::Item i1, QCollection::Item i2);
} ;

class Account
{
public:
	Account(const QString& i, const QString& n)
		: id(i), name(n), openingBalance(0.0)
	{
		kotrusId = "";
	}
	~Account() { };

	const QString& getId() const { return id; }
	const QString& getName() const { return name; }

	void setKotrusId(const QString& k) { kotrusId = k; }
	const QString& getKotrusId() const { return kotrusId; }

	void setOpeningBalance(double b)
	{
		openingBalance = b;
	}
	void book(Transaction* t)
	{
		transactions.inSort(t);
	}

	void balance(time_t d);

private:
	Account() { };	// don't use this
	QString id;
	QString name;
	QString kotrusId;
	double openingBalance;
	QList<Transaction> transactions;
} ;

class AccountList : public QList<Account>
{
public:
	AccountList() { }
	~AccountList() { }

	void addAccount(Account* a)
	{
		inSort(a);
	}
	Account* getAccount(const QString& id)
	{
		Account key(id, "");
		return at(find(&key));
	}
	
protected:
	virtual int compareItems(QCollection::Item i1, QCollection::Item i2);
} ;

#endif
