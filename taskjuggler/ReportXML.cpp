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
   
   QDomDocument doc( "TaskjugglerTasks" );
   QDomElement root = doc.createElement( "TaskJuggerTasks" );
   doc.appendChild( root );

   Task *task = project->taskListFirst();
   root.appendChild( task->xmlElement( doc ));
   while( (task = project->taskListNext()) != 0 )
   {
      root.appendChild( task->xmlElement( doc ));
   }	

   QString xml = doc.toString();

   // qDebug( "XML: %s", xml.latin1() );
   
}

