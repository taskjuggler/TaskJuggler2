/*
 * The TaskJuggler Project Management Software
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id: TjReport.cpp 1136 2005-08-13 02:49:59Z cs $
 */

#ifndef _TjObjPosTableEntry_h_
#define _TjObjPosTableEntry_h_

class CoreAttributes;

class TjObjPosTableEntry {
public:
    TjObjPosTableEntry(const CoreAttributes* c, int p, int h) :
        ca(c), pos(p), height(h) { }
    ~TjObjPosTableEntry() { }

    const CoreAttributes* getCoreAttributes() const { return ca; }
    int getPos() const { return pos; }
    int getHeight() const { return height; }

private:
    TjObjPosTableEntry() { }    // Don't use this

    const CoreAttributes* ca;
    int pos;
    int height;
} ;

#endif

