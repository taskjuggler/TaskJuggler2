/*
 * Report.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
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
	   Priority     CDATA #REQUIRED
	   Copyright    CDATA #REQUIRED>
 <!ELEMENT start        (#PCDATA)>
 <!ELEMENT end          (#PCDATA)>
 <!ELEMENT now          (#PCDATA)>

 <!ATTLIST start
           humanReadable CDATA #REQUIRED>
 <!ATTLIST end
           humanReadable CDATA #REQUIRED>
 <!ATTLIST now
           humanReadable CDATA #REQUIRED>

   /-- DTD --/
*/


#include <qfile.h>
#include <config.h>

#include "Project.h"
#include "ReportXML.h"
#include "Utility.h"

ReportXML::ReportXML(Project* p, const QString& f, time_t s, time_t e,
					 const QString& df, int dl) :
   Report(p, f, s, e, df, dl)
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
   // FIXME: All projectIDs need to be saved here.
   proj.setAttribute( "Id", project->getCurrentId());

   proj.appendChild( ReportXML::createXMLElem( doc, "Name", project->getName()));
   QString hStr = project->getVersion();
   if( !hStr.isEmpty() )
      proj.appendChild( ReportXML::createXMLElem( doc, "Version", hStr ));

   hStr = project->getCopyright();
   if( !hStr.isEmpty() )
      proj.appendChild( ReportXML::createXMLElem( doc, "Copyright", hStr ));
   
   proj.appendChild( ReportXML::createXMLElem( doc, "Priority",
					       QString::number(project->getPriority())));
   
   QDomElement tempElem;
   tempElem = ReportXML::createXMLElem( doc, "start",
					QString::number(project->getStart()));
   tempElem.setAttribute( "humanReadable", time2ISO( project->getStart()));
   
   proj.appendChild( tempElem );
   
   tempElem = ReportXML::createXMLElem( doc, "end",
					QString::number(project->getEnd()));
   tempElem.setAttribute( "humanReadable", time2ISO( project->getEnd()));
   proj.appendChild( tempElem );

   tempElem = ReportXML::createXMLElem( doc, "now",
					QString::number(project->getNow()));
   tempElem.setAttribute( "humanReadable", time2ISO( project->getNow()));
   proj.appendChild( tempElem );
   
   doc.appendChild( proj );

   /* retrieve Tasklist from the project ... */
   TaskList taskList = project->getTaskList();
   /* ...and sort it */
   sortTaskList( taskList );

   /* do a loop over all tasks */
   Task *task = taskList.first();
   proj.appendChild( task->xmlElement( doc ));
   while( (task = taskList.next()) != 0 )
   {
      /* only tasks which have child-tasks and no parent,
      *  since tasks do their subtasks recursive */
      if( task->isContainer() && task->getParent() == 0 )
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

