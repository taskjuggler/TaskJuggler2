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
        tcf(tcf_), tli(tli_), tci(tci_), rows(1), columns(1),
        rightPadding(-1), leftPadding(-1),
        boldText(FALSE), fontFactor(100) { }
    ~TableCellInfo() { }

    void setRows(int r) { rows = r; }
    int getRows() const { return rows; }

    void setColumns(int c) { columns = c; }
    int getColumns() const { return columns; }

    void setRightPadding(int rp) { rightPadding = rp; }
    int getRightPadding() const { return rightPadding; }

    void setLeftPadding(int lp) { leftPadding = lp; }
    int getLeftPadding() const { return leftPadding; }

    void setHAlign(const QString& a) { hAlign = a; }
    const QString& getHAlign() const { return hAlign; }

    void setBgColor(const QColor& bgc) { bgColor = bgc; }
    const QColor& getBgColor() const { return bgColor; }

    void setBoldText(bool b) { boldText = b; }
    bool getBoldText() const { return boldText; }

    void setFontFactor(int ff) { fontFactor = ff; }
    int getFontFactor() const { return fontFactor; }

    void setStatusText(const QString& s) { statusText = s; }
    const QString& getStatusText() const { return statusText; }

    void setToolTipID(const QString& s) { toolTipID = s; }
    const QString& getToolTipID() const { return toolTipID; }

    void setToolTipText(const QString& s) { toolTipText = s; }
    const QString& getToolTipText() const { return toolTipText; }

    TableColumnFormat* const tcf;
    TableLineInfo* const tli;
    TableColumnInfo* const tci;

private:
    int rows;
    int columns;
    int rightPadding;
    int leftPadding;
    QString hAlign;
    QColor bgColor;
    bool boldText;
    int fontFactor;
    QString statusText;
    QString toolTipID;
    QString toolTipText;
} ;

#endif

