/*
 * Allocation.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

class Allocation
{
public:
   Allocation(Resource* r);
   
   ~Allocation() { }
   
   Resource* getResource() const { return resource; }
   
   void setLoad(int l) { load = l; }
   int getLoad() const { return load; }
   
   void setPersistent(bool p) { persistent = p; }
   bool isPersistent() const { return persistent; }

   void setLockedResource(Resource* r) { lockedResource = r; }
   Resource* getLockedResource() const { return lockedResource; }

   void addAlternative(Resource* r) { alternatives.append(r); }
   Resource* first() { return alternatives.first(); }
   Resource* next() { return alternatives.next(); }

   QDomElement xmlElement( QDomDocument& doc ) const;
private:
   // Don't use this.
   Allocation();

   Resource* resource;
   // The maximum daily usage of the resource.
   int load;

   /* True if the allocation should be persistent over the whole task.
    * If set the first selection will not be changed even if there is an
    * available alternative. */
   bool persistent;

   Resource* lockedResource;
   
   // List of alternative resources.
   QList<Resource> alternatives;
} ;

