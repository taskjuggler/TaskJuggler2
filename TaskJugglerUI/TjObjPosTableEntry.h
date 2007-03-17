/*
 * The TaskJuggler Project Management Software
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _TjObjPosTableEntry_h_
#define _TjObjPosTableEntry_h_

class CoreAttributes;

class TjObjPosTableEntry {
public:
    TjObjPosTableEntry(CoreAttributes* c, CoreAttributes* sc,
                       int p, int h) : ca(c), subCA(sc), pos(p), height(h) { }
    ~TjObjPosTableEntry() { }

    CoreAttributes* getCoreAttributes() const { return ca; }
    CoreAttributes* getSubCoreAttributes() const { return subCA; }
    int getPos() const { return pos; }
    int getHeight() const { return height; }

private:
    CoreAttributes* ca;
    CoreAttributes* subCA;
    int pos;
    int height;
} ;

#endif

