/*
 * HTMLTaskReport.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _HTMLTaskReport_h_
#define _HTMLTaskReport_h_

#include "ReportHtml.h"
#include "Task.h"

class Project;

/**
 * @short Stores all information about an HTML task report.
 * @author Chris Schlaeger <cs@suse.de>
 */
class HTMLTaskReport : public ReportHtml
{
public:
	HTMLTaskReport(Project* p, const QString& f, time_t s, time_t e,
				   const QString& df, int dl);
	~HTMLTaskReport() { }

	bool generate();

private:
	HTMLTaskReport() { }
} ;

#endif
