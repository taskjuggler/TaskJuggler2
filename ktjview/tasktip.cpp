/*
 * tasktip.cpp - TaskJuggler Viewer
 *
 * Copyright (c) 2001, 2002 by Chris Schlaeger <cs@suse.de>
 *                             Klaas Freitag <freitag@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <klocale.h>

#include "tasktip.h"
#include "ktvcanvasitem.h"
#include "ktvtaskcanvasview.h"
#include "Utility.h"
#include "Task.h"


void TaskTip::maybeTip( const QPoint& pos )
{
   if ( !parentWidget()->inherits( "KTVTaskCanvasView" ) )
      return;

   KTVTaskCanvasView *viewPort = (KTVTaskCanvasView*) parentWidget();
   Q_CHECK_PTR(viewPort);

   KTVCanvasItemBase *cItem = viewPort->taskItemAt( pos );

   if( !cItem )  /* No item found */
   {
      return;
   }

   QRect r( cItem->rect() );
   QRect rView( viewPort->contentsToViewport( r.topLeft() ),
		viewPort->contentsToViewport( r.bottomRight() ));

   Task *mt = cItem->getTask();
   if( mt )
   {
       // qDebug( "The items rect starts is from %d to %d with task %p", r.x(), r.right(), mt );

      tip( rView, beautyTask(mt) );
   }

}


QString TaskTip::beautyTask( Task *t ) const
{
   QString ret = i18n( "Error: Can not retrieve task info");
   if( t )
   {
      QString h;

      ret = i18n( "Task <B>" ) + t->getName() + "</B><BR>";
      ret += QString("<TABLE cellpadding=\"0\" cellspacing=\"2\"><TR><TD>Plan Start</TD><TD>%1</TD></TR>").arg(time2ISO( t->getStart(Task::Plan) ));
      ret += QString("<TR><TD>Plan End</TD><TD>%1</TD></TR></TABLE>").arg(time2ISO(t->getEnd(Task::Plan) ));
   }
   return ret;
}
