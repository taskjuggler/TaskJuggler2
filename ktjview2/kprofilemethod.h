/* This file is part of the KDE project
   Copyright (C) 2002 David Faure <faure@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2, as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef KPROFILE_METHOD_H
#define KPROFILE_METHOD_H

#include <qdatetime.h>
#include <kdebug.h>

/**
 * Those macros help profiling using QTime.
 * They allow to sum up the time taken by a given bit of code
 * in a method called several times.
 * This way one can find out which low-level method used by a high-level
 * method is taking most of its time
 *
 *
 *  Declare the var somewhere, out of any method:
 *   int pr_theMethod = 0;
 *  (the name pr_* helps finding and removing all of this before committing)
 *
 *  Then in the method, around the code to be timed:
 *   PROFILE_METHOD_BEGIN( pr_theMethod );
 *   ...
 *   PROFILE_METHOD_END( pr_theMethod );
 *
 *  And finally, to see the result, put this call in a method
 *  called after all that (destructor, program exit...)
 *   PROFILE_METHOD_PRINT( pr_theMethod, "theMethod" );
 *
 */
#define PROFILE_METHOD_BEGIN(sym) extern int sym; QTime profile_dt##sym; profile_dt##sym.start();
#define PROFILE_METHOD_END(sym) extern int sym; sym += profile_dt##sym.elapsed();
#define PROFILE_METHOD_PRINT(sym, name) extern int sym; kdDebug() << name << " took " << sym << " milliseconds" << endl;

#endif // KPROFILE_METHOD_H
