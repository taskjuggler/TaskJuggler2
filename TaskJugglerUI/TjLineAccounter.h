/*
 * The TaskJuggler Project Management Software
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id: taskjuggler.cpp 1085 2005-06-23 20:34:54Z cs $
 */

#ifndef _TjLineAccounter_h_
#define _TjLineAccounter_h_

#include <list>

class TjLine
{
    friend class TjLineAccounter;
public:
    TjLine(int c, int s, int e) : coord(c), start(s), end(e) { }
    ~TjLine() { }

private:
    TjLine() { }

    int coord;
    int start;
    int end;
};

class TjLineAccounter
{
public:
    TjLineAccounter(int md) : minDist(md) { }
    ~TjLineAccounter();

    void insertLine(bool vertical, int coord, int start, int end);
    bool collision(bool vertical, int coord, int start, int end);

private:
    TjLineAccounter() { }

    int minDist;
    std::list<TjLine*> vertLines;
    std::list<TjLine*> horizLines;
} ;

#endif

