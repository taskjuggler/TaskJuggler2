/*
 * HTMLTaskReport.h - TaskJuggler
 *
 * Copyright (c) 2001 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _HTMLTaskReport_h_
#define _HTMLTaskReport_h_

#include "ReportHtml.h"

class Project;

class HTMLTaskReport : public ReportHtml
{
public:
	HTMLTaskReport(Project* p, const QString& f, time_t s, time_t e) :
		ReportHtml(p, f, s, e) { showActual = FALSE; }
	~HTMLTaskReport() { }

	bool generate();

	void setShowActual(bool s) { showActual = s; }

private:
	bool showActual;
	HTMLTaskReport() { }
} ;

#endif
