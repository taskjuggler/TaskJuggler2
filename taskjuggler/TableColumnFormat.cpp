/*
 * TableColumnFormat.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "TableColumnFormat.h"
#include "ReportElement.h"

TableColumnFormat::TableColumnFormat(ReportElement* e, const QString& t) :
  el(e), title(t)
{
    genHeadLine1 = &ReportElement::genHeadDefault;
    genHeadLine2 = 0;

    genTaskLine1 = &ReportElement::genCellEmpty;
    genTaskLine2 = 0;

    genResourceLine1 = &ReportElement::genCellEmpty;
    genResourceLine2 = 0;
}

