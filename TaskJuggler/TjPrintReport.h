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

#include "TaskList.h"
#include "ResourceList.h"

class QPaintDevice;
class Report;
class QtReportElement;
class TaskList;
class ReportElement;

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

    void computeTableMetrics();

    void printReportCell(TjReportRow* row, int col);

    const Report* reportDef;
    const QtReportElement* reportElement;

    int scenario;
    TaskList taskList;
    ResourceList resourceList;

    int maxDepthTaskList;
    int maxDepthResourceList;

    QPtrList<TjReportRow> rows;
    QPtrList<TjReportColumn> columns;

    QPaintDevice* paintDevice;
    QPainter p;

    // The top and left (non-printable) margin of the page in pixels.
    int topMargin;
    int leftMargin;

    // The printable size of the page in pixels.
    int pageWidth;
    int pageHeight;

    int cellMargin;
} ;

#endif
