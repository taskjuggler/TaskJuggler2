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

#include "TaskList.h"
#include "ResourceList.h"

class QPaintDevice;
class QPainter;
class Report;
class QtReportElement;
class TaskList;
class ReportElement;

class TjPrintReport
{
public:
    TjPrintReport(const Report* rd, QPaintDevice* pd);
    virtual ~TjPrintReport();

    virtual bool generate() = 0;

    void printReportPage(int x, int y);

    int getNumberOfColumns() const { return columns.count(); }

    void getNumberOfPages(int& xPages, int& yPages);

protected:
    void generateTableHeader(const ReportElement* el);
    void generateTaskListRow(const ReportElement* el, TjReportRow* row,
                             const Task* task, const Resource* resource = 0);
    void generateResourceListRow(const ReportElement* el, TjReportRow* row,
                                 const Resource* resource,
                                 const Task* task = 0);

    void generateCustomAttribute(const CoreAttributes* ca, const QString name,
                                 QString& cellText) const;

    void computeTableMetrics();

    const Report* reportDef;
    const QtReportElement* reportElement;

    QPaintDevice* paintDevice;
    int scenario;
    TaskList taskList;
    ResourceList resourceList;
    QPainter* p;

    int maxDepthTaskList;
    int maxDepthResourceList;

    QPtrList<TjReportRow> rows;
    QPtrList<TjReportColumn> columns;

    // The top and left (non-printable) margin of the page in pixels.
    int topMargin;
    int leftMargin;

    // The printable size of the page in pixels.
    int pageWidth;
    int pageHeight;
} ;

#endif
