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
   
   KTVCanvasItemBase *cItem = ((KTVTaskCanvasView*) parentWidget())->taskItemAt( pos );

   if( !cItem )  /* No item found */
   {
      return;
      qDebug("There is no item!" );
   }

   QRect r( cItem->rect() );
   qDebug( "The items rect starts at %d-%d", r.x(), r.y() );
   tip( r, beautyTask(cItem->getTask()) );

}


QString TaskTip::beautyTask( Task *t ) const
{
   QString ret = i18n( "Error: Can not retrieve task info");
   if( t )
   {
      QString h;
      
      ret = i18n( "Task " ) + t->getName() + " (" + t->getId() + ")<P>";
      ret += QString("<TABLE cellpadding=\"2\" cellspacing=\"2\"><TR><TD>Plan Start</TD><TD>%1</TD></TR>").arg(time2ISO( t->getPlanStart() ));
      ret += QString("<TR><TD>Plan End</TD><TD>%1</TD></TR></TABLE>").arg(time2ISO( t->getPlanEnd() ));
#if 0
      ret += i18n( "Plan: " ) +  );
      ret += " - " + time2ISO( t->getPlanEnd() )+ "<BR>";
      ret += i18n( "Actual: " ) + time2ISO( t->getActualStart() );
      ret += " - " + time2ISO( t->getActualEnd() )+ "<BR>";
#endif
   }
   return ret;
}
