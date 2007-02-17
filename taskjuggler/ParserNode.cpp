/*
 * ParserNode.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "ParserNode.h"
#include "ParserElement.h"

ParserNode::ParserNode(ParserElement* pEl) :
    parentElement(pEl),
    elements()
{
    elements.setAutoDelete(TRUE);
    if (pEl)
        pEl->setNode(this);
}

ParserNode::~ParserNode()
{
}

const ParserElement*
ParserNode::getElement(const QString& key) const
{
    return elements.find(key);
}

