/*
 * ReferenceAttribute.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef _ReferenceAttribute_h_
#define _ReferenceAttribute_h_

#include <qstring.h>

#include "CustomAttribute.h"

/*
 * @short User defined attribute that holds a text and a link.
 * @author Chris Schlaeger <cs@kde.org>
 */
class ReferenceAttribute : public CustomAttribute
{
public:
    ReferenceAttribute() { }
    ReferenceAttribute(const ReferenceAttribute& ra) :
        CustomAttribute(ra)
    {
        url = ra.url;
        label = ra.label;
    }
    ReferenceAttribute(const QString& u, const QString& l) :
        url(u), label(l) { }
    virtual ~ReferenceAttribute() { }

    CustomAttributeType getType() const { return CAT_Reference; }
    void setUrl(const QString& u) { url = u; }
    const QString& getURL() const { return url; }

    void setLabel(const QString& l) { label = l; }
    const QString& getLabel() const { return label; }

private:
    QString url;
    QString label;
} ;

#endif


