/*
 * ResourceList.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef _SbBooking_h_
#define _SbBooking_h_

#include <qstring.h>

#include "Account.h"
#include "Project.h"
#include "Task.h"

/**
 * @short Booking information for a time slot of the resource.
 * @author Chris Schlaeger <cs@suse.de>
 */
class SbBooking
{
public:
	SbBooking(Task* t, QString a = "", QString i = "")
		: task(t), account(a), projectId(i) { }
	~SbBooking() { }
	
	Task* getTask() const { return task; }

	void setAccount(const QString a) { account = a; }
	const QString& getAccount() const { return account; }

	void setProjectId(const QString i) { projectId = i; }
	const QString& getProjectId() const { return projectId; }

private:
	/// A pointer to the task that caused the booking
	Task* task;
	/// String identifying the KoTrus account the effort is credited to.
	QString account;
	/// The Project ID
	QString projectId;
};

#endif

