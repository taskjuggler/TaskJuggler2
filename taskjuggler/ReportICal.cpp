/*
 * ReportICal.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@suse.de>
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

bool
ReportICal::generate()
{
   KCal::CalendarLocal cal;
   qDebug( "Generate ical output to "+  fileName );

   cal.setEmail( "");

   TaskList filteredList;
   if (!filterTaskList(filteredList, 0))
       return FALSE;
   sortTaskList(filteredList);
 
    for (TaskListIterator tli(filteredList); *tli != 0; ++tli)
        if ((*tli)->isContainer() && (*tli)->getParent() == 0)
            addTask(*tli, &cal);    

   KCal::ICalFormat *format = new KCal::ICalFormat( ); // &cal );
   cal.save( fileName, format );
   qDebug( "saving ical to file " + fileName + " OK" );
   return TRUE;
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
      for (TaskListIterator tli(task->getSubListIterator()); *tli != 0; ++tli)
      {
     // qDebug("Turniing wheel" );
     
     if( *tli != task )
     {
        // qDebug("Adding subtask " + subTask->getName());
        KCal::Todo *subTodo = addATask( *tli, cal );
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
