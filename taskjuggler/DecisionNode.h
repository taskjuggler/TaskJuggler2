/*
 * DecisionNode.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _DecisionNode_h_
#define _DecisionNode_h_

#include <qstring.h>
#include <qptrlist.h>

class DecisionNode
{
public:
    DecisionNode(DecisionNode* p, const QString& t);
    ~DecisionNode() { }

    const QString& getTag() const { return tag; }

    bool getCompleted() const { return completed; }

    bool checkArc(const QString& t);

    DecisionNode* followArc(const QString& t);

    void terminateBranch(double r, bool minimize);

private:
    DecisionNode* parent;
    QString tag;
    double rating;
    bool completed;
    DecisionNode* bestArc;
    QPtrList<DecisionNode> arcs;
} ;

#endif

