/*
 * TableColumnFormat.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "TableColumnFormat.h"
#include "ReportElement.h"

TableColumnFormat::TableColumnFormat(const QString& i, ReportElement* e, 
                                     const QString& t) :
  id(i), el(e), title(t)
{
    genHeadLine1 = &ReportElement::genHeadDefault;
    genHeadLine2 = 0;

    genTaskLine1 = &ReportElement::genCellEmpty;
    genTaskLine2 = 0;

    genResourceLine1 = &ReportElement::genCellEmpty;
    genResourceLine2 = 0;

    genAccountLine1 = &ReportElement::genCellEmpty;
    genAccountLine2 = 0;

    genSummaryLine1 = &ReportElement::genCellEmpty;
    genSummaryLine2 = 0;

    fontFactor = 100;
    noWrap = FALSE;

    if (el)
        el->addColumnFormat(id, this);
}

