/*
 * The TaskJuggler Project Management Software
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id: TjReport.cpp 1169 2005-09-29 04:19:30Z cs $
 */

#ifndef _KPrinterWrapper_h_
#define _KPrinterWrapper_h_

#include <kprinter.h>

/**
 * The sole purpose of this class is to make KPrinter::preparePrinting
 * accessable to objects that use KPrinter. Orientation and page geometry
 * information are not passed to QPrinter if this function isn't called.
 */
class KPrinterWrapper : public KPrinter
{
public:
    KPrinterWrapper() : KPrinter(true, QPrinter::ScreenResolution) { }
    ~KPrinterWrapper() { }

    void preparePrinting() { KPrinter::preparePrinting(); }
} ;

#endif

