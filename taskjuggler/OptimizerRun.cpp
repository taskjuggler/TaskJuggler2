/*
 * OptimizerRun.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@suse.de>
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

OptimizerRun::OptimizerRun(Optimizer* o) : optimizer(o)
{
    currentNode = 0;
}

OptimizerRun::~OptimizerRun()
{
}

void 
OptimizerRun::terminate(double rating)
{
    if (!currentNode)
        return;

    currentNode->terminateBranch(rating, optimizer->getMinimize());
}

