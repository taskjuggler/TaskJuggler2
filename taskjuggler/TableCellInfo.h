/*
 * TableCellInfo.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _TableCellInfo_h_
#define _TableCellInfo_h_

class TableColumnFormat;
class TableLineInfo;
class TableColumnInfo;

class TableCellInfo
{
public:
    TableCellInfo(TableColumnFormat* tcf_, TableLineInfo* tli_,
                  TableColumnInfo* tci_) :
        tcf(tcf_), tli(tli_), tci(tci_) { }
    ~TableCellInfo() { }

    TableColumnFormat* tcf;
    TableLineInfo* tli;
    TableColumnInfo* tci;
} ;

#endif

