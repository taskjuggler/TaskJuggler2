/*
 * LoopDetectorInfo.h - TaskJuggler
 *
 * Copyright (c) 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _LoopDetectorInfo_h_
#define _LoopDetectorInfo_h_

class LoopDetectorInfo;

/**
 * This class stores information about a waypoint the dependency loop
 * detector passes when looking for loops. 
 *
 * @short Utility class for the dependency loop detector.
 * @author Chris Schlaeger <cs@suse.de>
 */
class LoopDetectorInfo
{
    friend class LDIList;
public:
    LoopDetectorInfo() { }
    LoopDetectorInfo(Task* _t, bool ae) : t(_t), atEnd(ae) { }
    ~LoopDetectorInfo() { }

    enum FromWhere { fromParent, fromSub, fromPrev, fromSucc, fromOtherEnd };

    bool operator==(const LoopDetectorInfo& ldi) const
    {
        return t == ldi.t && atEnd == ldi.atEnd;
    }
    bool operator!=(const LoopDetectorInfo& ldi) const
    {
        return t != ldi.t || atEnd != ldi.atEnd;
    }
    Task* getTask() const { return t; }
    bool getAtEnd() const { return atEnd; }
    LoopDetectorInfo* next() const { return nextLDI; }
    LoopDetectorInfo* prev() const { return prevLDI; }
protected:
    LoopDetectorInfo* nextLDI;
    LoopDetectorInfo* prevLDI;
private:
    Task* t;
    bool atEnd;
} ;

/**
 * This class stores the waypoints the dependency loop detector passes when
 * looking for loops. Since it is very performance critical we use a
 * handrolled list class instead of a Qt class. 
 *
 * @short Waypoint list of the dependency loop detector.
 * @author Chris Schlaeger <cs@suse.de>
 */
class LDIList
{
public:
    LDIList() 
    {
        root = leaf = 0;
        items = 0;   
    }
    virtual ~LDIList()
    {
        for (LoopDetectorInfo* p = root; p; p = root)
        {
            root = p->nextLDI;
            delete p;
        }
    }
    LoopDetectorInfo* first() const { return root; }
    LoopDetectorInfo* last() const { return leaf; }
    long count() const { return items; }
    
    void append(LoopDetectorInfo* p)
    {
        if (root == 0)
        {
            root = leaf = p;
            leaf->prevLDI = 0;
        }
        else
        {
            leaf->nextLDI = p;
            leaf->nextLDI->prevLDI = leaf;
            leaf = leaf->nextLDI;
        }
        leaf->nextLDI = 0;
        ++items;
    }
    void removeLast()
    {
        if (leaf == root)
        {
            delete leaf;
            root = leaf = 0;
        }
        else
        {
            leaf = leaf->prevLDI;
            delete leaf->nextLDI;
            leaf->nextLDI = 0;
        }
        --items;
    }
private:
    long items;
    LoopDetectorInfo* root;
    LoopDetectorInfo* leaf;
} ;

#endif

