/*
 * OptimizerRun.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "OptimizerRun.h"
#include "Optimizer.h"
#include "DecisionNode.h"
#include "debug.h"

OptimizerRun::OptimizerRun(Optimizer* o) :
    optimizer(o),
    currentNode(optimizer->getDecisionTreeRoot())
{
}

OptimizerRun::~OptimizerRun()
{
}

bool
OptimizerRun::checkArc(const QString& tag)
{
    if (DEBUGOP(10))
        qDebug("Checking arg %s of node %s", tag.latin1(),
               currentNode->getTag().latin1());

    return currentNode->checkArc(tag);
}

bool
OptimizerRun::followArc(const QString& tag)
{
    if (DEBUGOP(5))
        qDebug("Following arg %s of node %s", tag.latin1(),
               currentNode->getTag().latin1());

    DecisionNode* dn = currentNode->followArc(tag);
    if (dn)
    {
        currentNode = dn;
        return TRUE;
    }

    return FALSE;
}

void 
OptimizerRun::terminate(double rating)
{
    currentNode->terminateBranch(rating, optimizer->getMinimize());

    if (DEBUGOP(5))
        qDebug("Run was rated %f", rating);
}

