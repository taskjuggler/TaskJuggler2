/*
 * DecisionNode.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "DecisionNode.h"

DecisionNode::DecisionNode(DecisionNode* p, const QString& t)
    : parent(p), tag(t)
{
    rating = 0.0;
    completed = FALSE;
    arcs.setAutoDelete(TRUE);
    bestArc = 0;
}

DecisionNode::~DecisionNode()
{
}
    
bool 
DecisionNode::followArc(const QString& t)
{
    for (QPtrListIterator<DecisionNode> dni(arcs); *dni; ++dni)
        if ((*dni)->tag == t)
            return !(*dni)->completed;
   
    arcs.append(new DecisionNode(this, t));

    return TRUE;
}

void
DecisionNode::terminateBranch(double r, bool minimize)
{
    rating = r;
    completed = TRUE;
    
    if (parent)
    {
        bool allComleted = TRUE;
        double topRating = 0.0;
        for (QPtrListIterator<DecisionNode> dni(parent->arcs); *dni; ++dni)
        {
            if (!(*dni)->completed)
            {
                allComleted = FALSE;
                break;
            }
            if ((*dni)->rating > 0.0 && 
                ((minimize && (*dni)->rating < topRating) ||
                 (!minimize && (*dni)->rating > topRating)))
            {
                topRating = (*dni)->rating;
                parent->bestArc = *dni;
            }
        }
        if (allComleted)
            parent->terminateBranch(topRating, minimize);
    }
}

