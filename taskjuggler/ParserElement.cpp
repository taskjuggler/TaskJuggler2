/*
 * ParserElement.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "ParserElement.h"
#include "ParserNode.h"

ParserElement::ParserElement(const QString t, ParserFunctionPtr preF, 
                             ParserNode* pn, ParserFunctionPtr postF)
        : tag(t), preFunc(preF), node(0), postFunc(postF)
{
    pn->add(this, t);
}

ParserElement::~ParserElement()
{
    delete node;
}

