/*
 * Optimizer.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef _Optimizer_h_
#define _Optimizer_h_

#include <qptrlist.h>

class DecisionNode;
class OptimizerRun;

class Optimizer
{
public:
    Optimizer();
    ~Optimizer();

    bool getMinimize() const { return minimize; }

    bool optimumFound() const { return TRUE; }

    OptimizerRun* startNewRun();
    void finishRun(OptimizerRun* run);

private:
    DecisionNode* decisionTree;
    QPtrList<OptimizerRun> runs;
    bool minimize;
} ;

#endif

