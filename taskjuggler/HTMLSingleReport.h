/*
 * HTMLSingleReport.h - TaskJuggler
 *
 * Copyright (c) 2007 by Andreas Scherer <andreas_hacker@freenet.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id: $
 */

#ifndef _HTMLSingleReport_h_
#define _HTMLSingleReport_h_

#include "HTMLReport.h"
#include "HTMLReportElement.h"
#include "ElementHolder.h"

/**
 * @short Stores the common "m_element" data member.
 * @author Andreas Scherer <andreas_hacker@freenet.de>
 */
class HTMLSingleReport : public HTMLReport, public ElementHolder
{
public:
    HTMLSingleReport(Project* p, const QString& f, const QString& df, int dl) :
        HTMLReport(p, f, df, dl)
    { }

    virtual ~HTMLSingleReport() { }

    virtual bool generate()
    {
        if (!open())
            return false;

        generateHeader();
        bool ok = getTable()->generate();
        generateFooter();

        return close() && ok;
    }
} ;

#endif
