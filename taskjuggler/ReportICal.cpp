/*
 * ReportICal.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *                             Klaas Freitag <freitag@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <config.h>

#ifdef HAVE_KDE
#ifdef HAVE_ICAL

#include <qfile.h>
#include <config.h>
#include <libkcal/calendarlocal.h>

#include "Project.h"
#include "ReportICal.h"
#include "Utility.h"


ReportICal::ReportICal(Project* p, const QString& f, time_t s, time_t e) :
   Report(p, f, s, e, QString(), 0 )
{
   
}

void ReportICal::generate()
{
   KCal::CalendarLocal cal;
   qDebug( "Generate ical output to "+  fileName );

   cal.setEmail( "");

   TaskList filteredList;
   filterTaskList(filteredList, 0);
   sortTaskList(filteredList);
  
   TaskList taskList = filteredList; // project->getTaskList();
   Task *task = taskList.first();
   
   while( task )
   {
      if( task->isContainer() && task->getParent() == 0 )
      {
	 addATask( task, &cal );
      }
      task = taskList.next();
   }

   KCal::ICalFormat *format = new KCal::ICalFormat( ); // &cal );
   cal.save( fileName, format );
   qDebug( "saving ical to file " + fileName + " OK" );
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
	    // qDebug("Adding subtask " + subTask->getName());
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

#endif
#endif
