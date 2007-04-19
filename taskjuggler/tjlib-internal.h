/*
 * tjlib-internal.h - TaskJuggler
 *
 * Copyright (c) 2002 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef _tjlib_internal_h_
#define _tjlib_internal_h_

// TODO: For proper internationalization we have to fix this.
#ifndef I18N_NOOP
#define I18N_NOOP(x) (x)
#endif

#ifndef i18n
#define i18n(x) QString(x)
#endif

// Dummy macro to mark TaskJuggler syntax keywords
#define KW(a) a

#include <qstring.h>

inline void
tjWarning(const QString& msg)
{
    qWarning("%s", msg.latin1());
}

inline void
tjDebug(const QString& msg)
{
    qDebug("%s", msg.latin1());
}

#endif

