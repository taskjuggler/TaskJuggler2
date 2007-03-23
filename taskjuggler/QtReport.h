/*
 * QtReport.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _QtReport_h_
#define _QtReport_h_

#include "QtReportElement.h"
#include "Report.h"

#include <memory>

/**
 * @short Stores all information about a Qt report.
 * @author Chris Schlaeger <cs@kde.org>
 */
class QtReport : public Report
{
public:
    QtReport(Project* p, const QString& f, const QString& df, int dl) :
        Report(p, f, df, dl)
    {
        loadUnit = shortAuto;
    }

    virtual ~QtReport() { }

    virtual const char* getType() const { return "QtReport"; }

    void setTable(QtReportElement* element)
    {
        m_element.reset(element);
    }

    QtReportElement* getTable()
    {
        return m_element.get();
    }

    void generateHeader() { };
    void generateFooter() { };

    virtual bool generate()
    {
        return false;
    }

private:
    std::auto_ptr<QtReportElement> m_element;
} ;

#endif
