/*
 * Optimizer.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef _Optimizer_h_
#define _Optimizer_h_

#include "DecisionNode.h"

class OptimizerRun;

class Optimizer
{
public:
    Optimizer();
    ~Optimizer();

    DecisionNode* getDecisionTreeRoot() { return &decisionTree; }

    bool getMinimize() const { return minimize; }

    bool optimumFound() const;

    OptimizerRun* startNewRun();
    void finishRun(OptimizerRun* run);

private:
    DecisionNode decisionTree;
    QPtrList<OptimizerRun> runs;
    bool minimize;
} ;

#endif

