/*
 * ParserNode.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _ParserNode_
#define _ParserNode_

#include <qdict.h>

class ParserElement;

class ParserNode
{
public:
    ParserNode(ParserElement* parent = 0);
    ~ParserNode();

    void add(const ParserElement* pe, const QString& key)
    { 
        elements.insert(key, pe); 
    }

    const ParserElement* getElement(const QString& key) const;

private:
    QDict<const ParserElement> elements;
} ;

#endif

