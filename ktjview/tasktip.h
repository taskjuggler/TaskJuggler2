/*
 * tasktip.h - TaskJuggler Viewer
 *
 * Copyright (c) 2002 by Klaas Freitag <freitag@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _TASKTIP_H_
#define _TASKTIP_H_

#include <qtooltip.h>


class Task;

/**
 * Tooltip that displays task info. 
 */

class TaskTip: public QToolTip
{
public:
   TaskTip( QWidget *parent )
      : QToolTip( parent ) {  }
   QString beautyTask( Task *t ) const;
protected:
   void maybeTip( const QPoint& pos );

};

#endif
