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
#include <qptrdict.h>
#include <klocale.h>
#include <qfile.h>
#include <config.h>
#include <libkcal/calendarlocal.h>

#include "Project.h"
#include "ReportICal.h"
#include "Utility.h"
#include "ExpressionTree.h"
#include "Task.h"
#include "Operation.h"

ReportICal::ReportICal(Project* p, const QString& file, const QString& defFile, int dl) :
    Report(p, file, defFile, dl)
{
    taskSortCriteria[0] = CoreAttributesList::TreeMode;
    taskSortCriteria[1] = CoreAttributesList::StartUp;
    taskSortCriteria[2] = CoreAttributesList::EndUp;
    resourceSortCriteria[0] = CoreAttributesList::TreeMode;
    resourceSortCriteria[1] = CoreAttributesList::IdUp;
    scenarios.append(0);

}

bool
ReportICal::generate()
{
   KCal::CalendarLocal cal;
   qDebug( "Generate ical output to "+  fileName );

   if( !open()) {
       qWarning( i18n("Can not open ICal File for writing!"));
       return false;
   }
   
   TaskList filteredList;
   // show all tasks
   hideTask = new ExpressionTree(new Operation(0));
   // show all resources
   hideResource = new ExpressionTree(new Operation(0));

   if (!filterTaskList(filteredList, 0, getHideTask(), getRollUpTask() ))
       return FALSE;
   qDebug( "Anzahl der tasks: "+QString::number(filteredList.count()));
   sortTaskList(filteredList);

   QPtrDict<KCal::Todo> dict;

   /* First go over all tasks and create a todo for it. Store the todos
    * with task keys into a dict to create the relations later.
    */
   for (TaskListIterator tli(filteredList); *tli != 0; ++tli) {
       KCal::Todo *todo = addATask(*tli, &cal);
       dict.insert( *tli, todo );
   }

   /* Now go again over all tasks and create a relation in the todo
    * in case the task has a parent task. After that add the todo
    * to the calendar.
    */
   for(TaskListIterator tli(filteredList); *tli != 0; ++tli) {
       Task *t = *tli;
       if( t->getParent() && dict.find(t->getParent())) {
	   KCal::Todo *todo = dict[t];
	   // qDebug("Adding relation from "+t->getName()+" to " + t->getParent()->getName());
	   KCal::Todo *paTodo = dict[t->getParent()];
	   todo->setRelatedTo( paTodo );
       }
       cal.addTodo( dict[t] );
   }

   KCal::ICalFormat *format = new KCal::ICalFormat( );

   s << format->toString(&cal) << endl;
   f.close();
   
   qDebug( "saving ical to file " + fileName + " OK" );
   return TRUE;
}


KCal::Todo* ReportICal::addATask( Task *task, KCal::CalendarLocal *cal )
{
   if( !(task && cal) ) return 0;

   KCal::Todo *todo = new KCal::Todo();
   task->toTodo( todo, cal );

   return( todo );
}

#endif
#endif
