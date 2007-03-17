/*
 * ParserElement.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _ParserElement_h_
#define _ParserElement_h_

#include <qstring.h>

class QDomNode;
class ParserTreeContext;
class ParserElement;
class ParserNode;
class XMLFile;

typedef bool (XMLFile::*ParserFunctionPtr)(QDomNode&, ParserTreeContext&);

class ParserElement
{
public:
    ParserElement(const QString t, ParserFunctionPtr preF, ParserNode* n,
                  ParserFunctionPtr postF = 0);
    ~ParserElement();
   
    void setNode(ParserNode* n) { node = n; }
    const ParserNode* getNode() const { return node; }

    ParserFunctionPtr getPreFunc() const { return preFunc; }
    ParserFunctionPtr getPostFunc() const { return postFunc; }

    const QString& getTag() const { return tag; }

private:
    QString tag;
    /* Pointer to a function that is called before the elements of the node
     * are processed. */
    ParserFunctionPtr preFunc;
    ParserNode* node;
    /* Pointer to a function that is called after the elements of the node
     * have been processed. */
    ParserFunctionPtr postFunc;
} ;

#endif

