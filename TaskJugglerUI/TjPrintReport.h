/*
 * The TaskJuggler Project Management Software
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id: taskjuggler.cpp 1085 2005-06-23 20:34:54Z cs $
 */

#ifndef _TjPrintReport_h_
#define _TjPrintReport_h_

#include "TjReportRow.h"
#include "TjReportColumn.h"

#include <qptrlist.h>
#include <qpainter.h>
#include <qfont.h>
#include <qprinter.h>

#include "TaskList.h"
#include "ResourceList.h"

class QPaintDevice;
class Report;
class QtReportElement;
class TaskList;
class ReportElement;
class TjGanttChart;
class TjObjPosTable;

class TjPrintReport
{
public:
    TjPrintReport(const Report* rd, QPaintDevice* pd);
    virtual ~TjPrintReport();

    virtual void initialize() = 0;
    virtual bool generate() = 0;

    bool beginPrinting();
    void printReportPage(int x, int y);
    void endPrinting();

    int getNumberOfColumns() const { return columns.count(); }

    void getNumberOfPages(int& xPages, int& yPages);

protected:
    void generateTableHeader();
    void generateTaskListRow(TjReportRow* row, const Task* task,
                             const Resource* resource = 0);
    void generateResourceListRow(TjReportRow* row, const Resource* resource,
                                 const Task* task = 0);

    void generateCustomAttribute(const CoreAttributes* ca, const QString name,
                                 QString& cellText) const;

    void layoutPages();

    void printReportCell(TjReportRow* row, int col);

    const Report* reportDef;
    const QtReportElement* reportElement;

    int mmToXPixels(double mm);
    int mmToYPixels(double mm);
    int pointsToYPixels(double pts);

    int scenario;
    TaskList taskList;
    ResourceList resourceList;

    int maxDepthTaskList;
    int maxDepthResourceList;

    QPtrList<TjReportRow> rows;
    QPtrList<TjReportColumn> columns;

    TjObjPosTable* objPosTable;
    QPaintDevice* paintDevice;
    QPainter p;

    QFont standardFont;
    QFont tableHeaderFont;
    QFont headlineFont;
    QFont signatureFont;

    bool showGantt;
    TjGanttChart* ganttChart;
    QObject* ganttChartObj;

    // The top and left (non-printable) margin of the page in pixels.
    int topMargin;
    int leftMargin;

    // The printable size of the page in pixels
    int pageWidth;
    int pageHeight;

    // The leftmost pixes of the headline
    int headlineX;
    // The Y coordinate of the headline baseline
    int headlineBase;
    // The height of the headline in pixels
    int headlineHeight;

    // The top pixel of the table header
    int headerY;

    // The height of the table header in pixels
    int headerHeight;

    // Rightmost pixel of the table
    int tableRight;
    // Lowermost pixel of the table
    int tableBottom;

    // The margin around cell content in pixels
    int cellMargin;

    // The step size for indentation of table cell content in pixels
    int indentSteps;

    // The top pixel of the footer
    int footerY;
    // The height of the footer in pixels
    int footerHeight;

    // The top pixel of the bottom line.
    int bottomlineY;
    // The height of the bottom line in pixels
    int bottomlineHeight;

    QDict<int> specialColumns;
} ;

#endif
