/*
 * Allocation.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "Allocation.h"

#include "tjlib-internal.h"
#include "Resource.h"
#include "ResourceTreeIterator.h"
#include "ReportXML.h"
#include "UsageLimits.h"

/* -- DTD --
    <!ELEMENT Allocation (Load, Persistent)>
    <!ELEMENT Load       (#PCDATA)>
    <!ELEMENT Persistent (#PCDATA)>
    <!ATTLIST Allocation ResourceID CDATA #REQUIRED>
  /-- DTD --/
*/

Allocation::Allocation() :
    limits(0),
    shifts(),
    persistent(FALSE),
    mandatory(FALSE),
    lockedResource(0),
    conflictStart(0),
    candidates(),
    selectionMode(minAllocationProbability)
{
    shifts.setAutoDelete(TRUE);
}

Allocation::~Allocation()
{
    delete limits;
}

Allocation::Allocation(const Allocation& a) :
    limits(a.limits ? new UsageLimits(*a.limits) : 0),
    shifts(),
    persistent(a.persistent),
    mandatory(a.mandatory),
    lockedResource(a.lockedResource),
    conflictStart(0),
    candidates(a.candidates),
    selectionMode(a.selectionMode)
{
    shifts.setAutoDelete(TRUE);

    for (QPtrListIterator<ShiftSelection> sli(a.shifts); *sli; ++sli)
        shifts.append(new ShiftSelection(**sli));
}

void
Allocation::setLimits(UsageLimits* l)
{
    delete limits;
    limits = l;
}

bool
Allocation::isWorker() const
{
    /* For an allocation to be a worker, all allocated resources must have an
     * non zero efficiency. */
    for (QPtrListIterator<Resource> cli(candidates); *cli; ++cli)
        if (!(*cli)->isWorker())
            return false;

    return true;
}

/* Creation of the XML Reprsentation of the Allocation */
QDomElement Allocation::xmlElement( QDomDocument& doc )
{
   QDomElement elem = doc.createElement( "Allocation" );
   elem.appendChild(ReportXML::createXMLElem( doc, "Persistent", isPersistent() ? "Yes":"No" ));
   elem.setAttribute( "ResourceID", candidates.getFirst()->getId());

   /* candidates are missing TODO */
   return elem;

}

bool
Allocation::setSelectionMode(const QString& smt)
{
    if (smt == KW("order"))
        selectionMode = order;
    else if (smt == KW("minallocated"))
        selectionMode = minAllocationProbability;
    else if (smt == KW("minloaded"))
        selectionMode = minLoaded;
    else if (smt == KW("maxloaded"))
        selectionMode = maxLoaded;
    else if (smt == KW("random"))
        selectionMode = random;
    else
        return FALSE;
    return TRUE;
}

