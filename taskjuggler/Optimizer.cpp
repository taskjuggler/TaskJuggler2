/*
 * Optimizer.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "Optimizer.h"
#include "OptimizerRun.h"
#include "debug.h"

Optimizer::Optimizer() :
    decisionTree(0, "*Root*"),
    runs(),
    minimize(TRUE)
{
    runs.setAutoDelete(TRUE);
}

Optimizer::~Optimizer()
{
}

bool
Optimizer::optimumFound() const 
{
    return decisionTree.getCompleted();
}

OptimizerRun*
Optimizer::startNewRun()
{
    OptimizerRun* run = new OptimizerRun(this);
    runs.append(run);

    return run;
}

void
Optimizer::finishRun(OptimizerRun* run)
{
    runs.remove(run);
}
