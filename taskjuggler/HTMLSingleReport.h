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

#include <memory>

/**
 * @short Stores the common "m_element" data member.
 * @author Andreas Scherer <andreas_hacker@freenet.de>
 */
class HTMLSingleReport : public HTMLReport
{
public:
    HTMLSingleReport(Project* p, const QString& f, const QString& df, int dl) :
        HTMLReport(p, f, df, dl)
    { }

    virtual ~HTMLSingleReport() { }

    void setTable(HTMLReportElement* element)
    {
        m_element.reset(element);
    }

    HTMLReportElement* getTable()
    {
        return m_element.get();
    }

    void generateBody()
    {
        getTable()->generate();
    }

    virtual bool generate()
    {
        if (!open())
            return false;

        generateHeader();
        generateBody();
        generateFooter();

        return close();
    }

private:
    std::auto_ptr<HTMLReportElement> m_element;
} ;

#endif
