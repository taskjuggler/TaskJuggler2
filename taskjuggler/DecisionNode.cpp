/*
 * DecisionNode.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "DecisionNode.h"
#include "debug.h"

DecisionNode::DecisionNode(DecisionNode* p, const QString& t) :
    parent(p),
    tag(t),
    rating(0.0),
    completed(false),
    bestArc(0),
    arcs()
{
    arcs.setAutoDelete(true);
}

bool
DecisionNode::checkArc(const QString& t)
{
    if (completed)
        return true;

    for (QPtrListIterator<DecisionNode> dni(arcs); *dni; ++dni)
        if ((*dni)->tag == t)
            return !(*dni)->completed;
   
    arcs.append(new DecisionNode(this, t));
    return true;
}

DecisionNode* 
DecisionNode::followArc(const QString& t)
{
    if (completed)
        return bestArc;

    for (QPtrListIterator<DecisionNode> dni(arcs); *dni; ++dni)
        if ((*dni)->tag == t)
            return (*dni)->completed ? 0 : *dni;
   
    qFatal("Trying to follow non existing arc %s", t.latin1());
    return 0;
}

void
DecisionNode::terminateBranch(double r, bool minimize)
{
    if (arcs.isEmpty())
    {
        if (DEBUGOP(5))
            qDebug("Completing leaf node %s with rating %f", tag.latin1(), r);
        rating = r;
        completed = true;
    }
    else
    {
        bool allComleted = true;
        double topRating = 0.0;
        for (QPtrListIterator<DecisionNode> dni(arcs); *dni; ++dni)
        {
            if (!(*dni)->completed)
            {
                if (DEBUGOP(5))
                    qDebug("%s not yet completed", (*dni)->tag.latin1());
                allComleted = false;
                break;
            }
            if (topRating == 0.0 || 
                ((minimize && (*dni)->rating < topRating) ||
                 (!minimize && (*dni)->rating > topRating)))
            {
                topRating = (*dni)->rating;
                bestArc = *dni;
            }
        }
        if (allComleted)
        {
            if (DEBUGOP(5))
                qDebug("Completing node %s with rating %f", tag.latin1(),
                       topRating);
            rating = topRating;
            completed = true;
        }
    }

    if (completed && parent)
    {
        if (DEBUGOP(5))
            qDebug("Checking parent %s", parent->tag.latin1());
        parent->terminateBranch(0.0, minimize);
    }
}

