/*
 * Allocation.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "ResourceList.h"
#include "Allocation.h"
#include "ReportXML.h"


/* -- DTD --
    <!ELEMENT Allocation (Load, Persistent)>
    <!ELEMENT Load       (#PCDATA)>
    <!ELEMENT Persistent (#PCDATA)>
    <!ATTLIST Allocation ResourceID CDATA #REQUIRED>
  /-- DTD --/

*/

/* Constructor */
Allocation::Allocation( Resource *r):	
   resource(r),
   load(100),
   persistent( FALSE ),
   lockedResource(0)
{

}


/* Creation of the XML Reprsentation of the Allocation */
QDomElement Allocation::xmlElement( QDomDocument& doc ) const
{
   QDomElement elem = doc.createElement( "Allocation" );
   elem.appendChild(ReportXML::createXMLElem( doc, "Load", QString::number(getLoad()) ));
   elem.appendChild(ReportXML::createXMLElem( doc, "Persistent", isPersistent() ? "Yes":"No" ));
   elem.setAttribute( "ResourceID", resource->getId());
   
   /* Alternatives are missing TODO */
   return elem;

}
