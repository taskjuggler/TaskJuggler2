/*
 * ResourceList.h - TaskJuggler
 *
 * Copyright (c) 2001 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _FlagList_h_
#define _FlagList_h_

#include "qstringlist.h"

class FlagList : public QStringList
{
public:
	FlagList() { }
	~FlagList() { }

	void addFlag(QString flag)
	{
		if (!hasFlag(flag))
			append(flag);
	}
	void clearFlag(const QString& flag)
	{
		remove(flag);
	}
	bool hasFlag(const QString& flag)
	{
		return contains(flag) > 0;
	}

} ;

#endif
