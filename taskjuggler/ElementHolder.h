/*
 * ElementHolder.h - TaskJuggler
 *
 * Copyright (c) 2007 by Andreas Scherer <andreas_hacker@freenet.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id: $
 */

#ifndef _ElementHolder_h_
#define _ElementHolder_h_

#include "ReportElement.h"

#include <memory>

/**
 * @short Stores the common "m_element" data member.
 * @author Andreas Scherer <andreas_hacker@freenet.de>
 */
class ElementHolder
{
public:
    ElementHolder()
    { }

    virtual ~ElementHolder() { }

    void setTable(ReportElement* element)
    {
        m_element.reset(element);
    }

    ReportElement* getTable()
    {
        return m_element.get();
    }

private:
    std::auto_ptr<ReportElement> m_element;
} ;

#endif
