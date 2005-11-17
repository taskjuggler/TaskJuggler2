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


#include "pref.h"

#include <klocale.h>

#include <qlayout.h>
#include <qlabel.h>

TaskJugglerPreferences::TaskJugglerPreferences()
    : KDialogBase(TreeList, i18n("TaskJuggler Preferences"),
                  Help|Default|Ok|Apply|Cancel, Ok)
{
    // this is the base class for your preferences dialog.  it is now
    // a Treelist dialog.. but there are a number of other
    // possibilities (including Tab, Swallow, and just Plain)
    QFrame *frame;
    frame = addPage(i18n("First Page"), i18n("Page One Options"));
    m_pageOne = new TaskJugglerPrefPageOne(frame);

    frame = addPage(i18n("Second Page"), i18n("Page Two Options"));
    m_pageTwo = new TaskJugglerPrefPageTwo(frame);
}

TaskJugglerPrefPageOne::TaskJugglerPrefPageOne(QWidget *parent)
    : QFrame(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setAutoAdd(true);

    new QLabel(i18n("Add something here"), this);
}

TaskJugglerPrefPageTwo::TaskJugglerPrefPageTwo(QWidget *parent)
    : QFrame(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setAutoAdd(true);

    new QLabel(i18n("Add something here"), this);
}
#include "pref.moc"
