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

#include <qfile.h>
#include <config.h>
#include <libkcal/calendarlocal.h>

#include "Project.h"
#include "ReportICal.h"
#include "Utility.h"


ReportICal::ReportICal(Project* p, const QString& f, time_t s, time_t e) :
   Report(p, f, s, e )
{
   
}

void ReportICal::generate()
{
   KCal::CalendarLocal cal;
   qDebug( "Generate ical output to "+  fileName );

   cal.setEmail( "Klaas@suse.de");

   TaskList filteredList;
   filterTaskList(filteredList, 0);
   sortTaskList(filteredList);
  
   TaskList taskList = filteredList; // project->getTaskList();
   Task *task = taskList.first();
   while( (task = taskList.next()) != 0 )
   {
      if( task->isContainer())
	 addATask( task, &cal );
   }

   qDebug( "saving ical to "+  fileName );

   KCal::ICalFormat *format = new KCal::ICalFormat( &cal );
   cal.save( fileName, format );
   qDebug( "saving ical OK" );
}


KCal::Todo* ReportICal::addATask( Task *task, KCal::CalendarLocal *cal )
{
   if( !(task && cal) ) return 0;

   KCal::Todo *todo = new KCal::Todo();
   task->toTodo( todo, cal );
   cal->addTodo( todo );
   // qDebug("--- OK --- for " + task->getName() );

   if( task->isContainer() )
   {
      /* Task is has subtasks */
      TaskList subs;
      task->getSubTaskList(subs);
      for (Task* subTask = subs.first(); subTask != 0; subTask = subs.next())
      {
	 // qDebug("Turniing wheel" );
	 
	 if( subTask && subTask != task )
	 {
	    qDebug("Adding subtask " + subTask->getName());
	    KCal::Todo *subTodo = addATask( subTask, cal );
	    subTodo->setRelatedTo( todo );
	    // qDebug("Added subtask OK" );

	    // todo->setRelatedTo( subTodo );
	    // cal->addTodo( subTodo );
	 }
      }
   }
   return( todo );
}


