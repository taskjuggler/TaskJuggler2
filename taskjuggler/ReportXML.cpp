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
#include <qfile.h>
#include <config.h>

#include "Project.h"
#include "ReportXML.h"
#include "Utility.h"

ReportXML::ReportXML(Project* p, const QString& f, time_t s, time_t e) :
   Report(p, f, s, e )
{

}


void ReportXML::generate()
{
   if( ! project ) return;
   QDomDocument doc( "TaskJugglerTasks" );
   QDomElement root = doc.createElement( "TaskJugglerTasks" );
   doc.appendChild( root );

   Task *task = project->taskListFirst();
   root.appendChild( task->xmlElement( doc ));
   while( (task = project->taskListNext()) != 0 )
   {
      root.appendChild( task->xmlElement( doc ));
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

