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
    <!ELEMENT Allocation (#PCDATA)>
    <!ATTLIST Allocation ResourceID, #REQUIRED>
  /-- DTD --/

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
   QDomElement elem = ReportXML::createXMLElem( doc, "Allocation", QString::number(load) );
   elem.setAttribute( "ResourceID", resource->getId());
   
   /* Alternatives are missing TODO */
   return elem;

}
