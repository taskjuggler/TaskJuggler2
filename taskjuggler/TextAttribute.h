/*
 * TextAttribute.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef _TextAttribute_h_
#define _TextAttribute_h_

#include <qstring.h>

#include "CustomAttribute.h"

/*
 * @short User defined attribute that holds a text and a link.
 * @author Chris Schlaeger <cs@suse.de>
 */
class TextAttribute : public CustomAttribute
{
public:
    TextAttribute() { }
    TextAttribute(const TextAttribute& ta) :
        CustomAttribute(ta)
    {
        text = ta.text;
    }
    TextAttribute(const QString& t) { text = t; }
    virtual ~TextAttribute() { }

    CustomAttributeType getType() const { return CAT_Text; }

    void setText(const QString& t) { text = t; }
    const QString& getText() const { return text; }

private:
    QString text;
} ;

#endif


