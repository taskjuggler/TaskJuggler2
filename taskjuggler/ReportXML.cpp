/*
 * Report.cpp - TaskJuggler
 *
 * Copyright (c) 2001 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */


/* -- DTD --

 <!-- The head of all: Project -->
 <!ELEMENT Project      (start, end, now, Task+)>
 <!ATTLIST Project
           Name         CDATA #REQUIRED
	   Id           CDATA #REQUIRED
	   Version      CDATA #REQUIRED
	   Copyright    CDATA #REQUIRED>
 <!ELEMENT start        (#PCDATA)>
 <!ELEMENT end          (#PCDATA)>
 <!ELEMENT now          (#PCDATA)>
   /-- DTD --/
*/


#include <qfile.h>
#include <config.h>

#include "Project.h"
#include "ReportXML.h"
#include "Utility.h"

ReportXML::ReportXML(Project* p, const QString& f, time_t s, time_t e) :
   Report(p, f, s, e )
{

}


QDomElement ReportXML::createXMLElem( QDomDocument& doc, const QString& name, const QString& val )
{
   QDomElement elem = doc.createElement( name );
   QDomText t=doc.createTextNode( val );

   elem.appendChild( t );

   return( elem );
}



void ReportXML::generate()
{
   if( ! project ) return;
   QDomDocument doc( "Project" );

   /* Create the Project xml representation */
   QDomElement proj = doc.createElement( "Project" );
   proj.setAttribute( "Name", project->getName());
   proj.setAttribute( "Id", project->getId());
   proj.setAttribute( "Version", project->getVersion());
   proj.setAttribute( "Copyright", project->getCopyright());

   proj.appendChild( ReportXML::createXMLElem( doc, "start",
					       QString::number(project->getStart())));
   proj.appendChild( ReportXML::createXMLElem( doc, "end",
   					       QString::number(project->getEnd())));
   proj.appendChild( ReportXML::createXMLElem( doc, "now",
   					       QString::number(project->getNow())));

   doc.appendChild( proj );

   TaskList taskList = project->getTaskList();
   Task *task = taskList.first();
   proj.appendChild( task->xmlElement( doc ));
   while( (task = taskList.next()) != 0 )
   {
      proj.appendChild( task->xmlElement( doc ));
   }	

   QString xml = doc.toString();

   if( ! fileName.isEmpty())
   {
      QFile fi( fileName );
      if ( fi.open(IO_WriteOnly) ) {    // file opened successfully
        QTextStream t( &fi );        // use a text stream
	t << xml;
	fi.close();
      }
   }
   // qDebug( "XML: %s", xml.latin1() );
   
}

