/*
 * OptimizerRun.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _OptimizerRun_h_
#define _OptimizerRun_h_

#include <qstring.h>

class Optimizer;
class DecisionNode;

class OptimizerRun
{
public:
    OptimizerRun(Optimizer* o);
    ~OptimizerRun();

    void terminate(double rating);

    bool checkArc(const QString& tag);

    bool followArc(const QString& tag);

private:
    const Optimizer* optimizer;
    DecisionNode* currentNode;
} ;
        
#endif

