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

#include "qdom.h"

#include "tjlib-internal.h"
#include "Resource.h"
#include "ResourceTreeIterator.h"
#include "Allocation.h"
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
    lockedResource(0)
{
    shifts.setAutoDelete(TRUE);
    selectionMode = minAllocationProbability;
    limits = 0;
    persistent = mandatory = FALSE;
}

Allocation::~Allocation()
{
    delete limits;
}

Allocation::Allocation(const Allocation& a)
{
    shifts.setAutoDelete(TRUE);

    persistent = a.persistent;
    mandatory = a.mandatory;
    lockedResource = a.lockedResource;
    selectionMode = a.selectionMode;

    for (QPtrListIterator<ShiftSelection> sli(a.shifts); *sli; ++sli)
        shifts.append(new ShiftSelection(**sli));

    candidates = a.candidates;
    if (a.limits)
        limits = new UsageLimits(*a.limits);
    else
        limits = 0;
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

