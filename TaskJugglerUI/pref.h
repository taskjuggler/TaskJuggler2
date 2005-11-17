/*
 * The TaskJuggler Project Management Software
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */


#ifndef _TASKJUGGLERPREF_H_
#define _TASKJUGGLERPREF_H_

#include <kdialogbase.h>
#include <qframe.h>

class TaskJugglerPrefPageOne;
class TaskJugglerPrefPageTwo;

class TaskJugglerPreferences : public KDialogBase
{
    Q_OBJECT
public:
    TaskJugglerPreferences();

private:
    TaskJugglerPrefPageOne *m_pageOne;
    TaskJugglerPrefPageTwo *m_pageTwo;
};

class TaskJugglerPrefPageOne : public QFrame
{
    Q_OBJECT
public:
    TaskJugglerPrefPageOne(QWidget *parent = 0);
};

class TaskJugglerPrefPageTwo : public QFrame
{
    Q_OBJECT
public:
    TaskJugglerPrefPageTwo(QWidget *parent = 0);
};

#endif // _TASKJUGGLERPREF_H_
