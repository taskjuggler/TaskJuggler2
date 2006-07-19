/*
 * The TaskJuggler Project Management Software
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "TjReportCell.h"

TjReportCell::TjReportCell(TjReportRow* r, TjReportColumn* c) :
    row(r), column(c)
{
    indentLevel = 0;
}
