/*
 * QtReportElement.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "QtReportElement.h"

#include <config.h>

#include "TjMessageHandler.h"
#include "tjlib-internal.h"
#include "Project.h"
#include "Task.h"
#include "Resource.h"
#include "Account.h"
#include "Report.h"
#include "Booking.h"
#include "BookingList.h"
#include "Utility.h"
#include "MacroTable.h"
#include "TableColumnFormat.h"
#include "TableLineInfo.h"
#include "TableColumnInfo.h"
#include "TableCellInfo.h"
#include "ReferenceAttribute.h"
#include "TextAttribute.h"
#include "QtReport.h"
#include "UsageLimits.h"

QtReportElement::QtReportElement(Report* r, const QString& df, int dl) :
   ReportElement(r, df, dl)
{
}

