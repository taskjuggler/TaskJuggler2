/*
 * ParserElement.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
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
class ParserNode;
class XMLFile;

typedef bool (XMLFile::*ParserFunctionPtr)(QDomNode&, ParserTreeContext&);

class ParserElement
{
public:
    ParserElement(const QString t, ParserFunctionPtr f, ParserNode* n);
    ~ParserElement();
   
    void setNode(ParserNode* n) { node = n; }
    const ParserNode* getNode() const { return node; }

    ParserFunctionPtr getFunc() const { return func; }

    const QString& getTag() const { return tag; }

private:
    ParserElement();    // Don't use!

    QString tag;
    ParserFunctionPtr func;
    ParserNode* node;
} ;

#endif

