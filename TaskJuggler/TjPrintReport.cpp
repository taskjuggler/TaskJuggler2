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

#include "TjPrintReport.h"

#include <qpainter.h>
#include <qfontmetrics.h>

#include "Report.h"
#include "ReportElement.h"
#include "TableColumnInfo.h"
#include "ReportElement.h"
#include "Project.h"
#include "Task.h"
#include "Resource.h"
#include "TjReportCell.h"
#include "TjReportRow.h"
#include "TjReportColumn.h"
#include "TextAttribute.h"
#include "ReferenceAttribute.h"

TjPrintReport::TjPrintReport(const Report* rd) : reportDef(rd)
{
    rows.setAutoDelete(TRUE);
    columns.setAutoDelete(TRUE);

    p = new QPainter();

    leftMargin = topMargin = pageWidth = pageHeight = 0;
}

TjPrintReport::~TjPrintReport()
{
    delete p;
}

void
TjPrintReport::generateTableHeader(const ReportElement* reportElement)
{
    for (QPtrListIterator<TableColumnInfo>
         ci = reportElement->getColumnsIterator(); *ci; ++ci)
    {
        //const TableColumnFormat* tcf =
        //    reportElement->getColumnFormat((*ci)->getName());

        TjReportColumn* col = new TjReportColumn;
        columns.append(col);
    }
}

void
TjPrintReport::generateTaskListRow(const ReportElement* reportElement,
                                   TjReportRow* row, const Task* task,
                                   const Resource* resource)
{
    // Skip the first colum. It contains the hardwired task name.
    int colIdx= 0;
    for (QPtrListIterator<TableColumnInfo>
         ci = reportElement->getColumnsIterator(); *ci; ++ci, ++colIdx)
    {
        QString cellText;

        const TableColumnFormat* tcf =
            reportElement->getColumnFormat((*ci)->getName());

        if ((*ci)->getName() == "completed")
        {
            if (task->getCompletionDegree(scenario) ==
                task->getCalcedCompletionDegree(scenario))
            {
                cellText = QString("%1%")
                    .arg((int) task->getCompletionDegree(scenario));
            }
            else
            {
                cellText = QString("%1% (%2%)")
                    .arg((int) task->getCompletionDegree(scenario))
                    .arg((int) task->getCalcedCompletionDegree(scenario));
            }
        }
        else if ((*ci)->getName() == "cost")
        {
            double val = task->getCredits
                (scenario, Interval(reportElement->getStart(),
                                    reportElement->getEnd()), Cost, resource);
            cellText = tcf->realFormat.format(val, FALSE);
        }
        else if ((*ci)->getName() == "criticalness")
        {
            cellText = QString().sprintf("%f", task->getCriticalness(scenario));
        }
        else if ((*ci)->getName() == "duration")
            cellText = reportElement->scaledLoad
                (task->getCalcDuration(scenario), tcf->realFormat);
        else if ((*ci)->getName() == "effort")
        {
            double val = 0.0;
            val = task->getLoad(scenario, Interval(task->getStart(scenario),
                                                   task->getEnd(scenario)),
                                resource);
            cellText = reportElement->scaledLoad (val, tcf->realFormat);
        }
        else if ((*ci)->getName() == "end")
            cellText = time2user((task->isMilestone() ? 1 : 0) +
                                 task->getEnd(scenario),
                                 reportDef->getTimeFormat());
        else if ((*ci)->getName() == "endbuffer")
            cellText.sprintf("%3.0f", task->getEndBuffer(scenario));
        else if ((*ci)->getName() == "endbufferstart")
            cellText = time2user(task->getEndBufferStart(scenario),
                                 reportDef->getTimeFormat());
        else if ((*ci)->getName() == "id")
            cellText = task->getId();
        else if ((*ci)->getName() == "maxend")
            cellText = time2user(task->getMaxEnd(scenario),
                                 reportDef->getTimeFormat());
        else if ((*ci)->getName() == "maxstart")
            cellText = time2user(task->getMaxStart(scenario),
                                 reportDef->getTimeFormat());
        else if ((*ci)->getName() == "minend")
            cellText = time2user(task->getMinEnd(scenario),
                                 reportDef->getTimeFormat());
        else if ((*ci)->getName() == "minstart")
            cellText = time2user(task->getMinStart(scenario),
                                 reportDef->getTimeFormat());
        else if ((*ci)->getName() == "note" && !task->getNote().isEmpty())
            cellText = task->getNote();
        else if ((*ci)->getName() == "pathcriticalness")
            cellText =
                QString().sprintf("%f", task->getPathCriticalness(scenario));
        else if ((*ci)->getName() == "priority")
            cellText = QString().sprintf("%d", task->getPriority());
        else if ((*ci)->getName() == "projectid")
            cellText = task->getProjectId() + " (" +
                reportElement->getReport()->getProject()->getIdIndex
                (task->getProjectId()) + ")";
        else if ((*ci)->getName() == "profit")
        {
            double val = task->getCredits
                (scenario, Interval(reportElement->getStart(),
                                    reportElement->getEnd()), Revenue,
                 resource) - task->getCredits
                (scenario, Interval(reportElement->getStart(),
                                    reportElement->getEnd()), Cost, resource);
            cellText = tcf->realFormat.format(val, FALSE);
        }
        else if ((*ci)->getName() == "resources")
        {
            for (ResourceListIterator rli
                 (task->getBookedResourcesIterator(scenario)); *rli != 0; ++rli)
            {
                if (!cellText.isEmpty())
                    cellText += ", ";

                cellText += (*rli)->getName();
            }
        }
        else if ((*ci)->getName() == "responsible")
            cellText = task->getResponsible()->getName();
        else if ((*ci)->getName() == "revenue")
        {
            double val = task->getCredits
                (scenario, Interval(reportElement->getStart(),
                                    reportElement->getEnd()), Revenue,
                 resource);
            cellText = tcf->realFormat.format(val, FALSE);
        }
        else if ((*ci)->getName() == "start")
            cellText = time2user(task->getStart(scenario),
                                 reportDef->getTimeFormat());
        else if ((*ci)->getName() == "startbuffer")
            cellText.sprintf("%3.0f", task->getStartBuffer(scenario));
        else if ((*ci)->getName() == "startbufferend")
            cellText = time2user(task->getStartBufferEnd(scenario),
                                 reportDef->getTimeFormat());
        else if ((*ci)->getName() == "status")
            cellText = task->getStatusText(scenario);
        else if ((*ci)->getName() == "statusnote")
            cellText = task->getStatusNote(scenario);
        else
            generateCustomAttribute(task, (*ci)->getName(), cellText);

        TjReportCell* cell = new TjReportCell(row, columns.at(colIdx));
        cell->setText(cellText);
        row->insertCell(cell, colIdx);
    }
}

void
TjPrintReport::generateResourceListRow(const ReportElement* /*reportElement*/,
                                       TjReportRow* /*row*/,
                                       const Resource* /*resource*/,
                                       const Task* /*task*/)
{
}

void
TjPrintReport::generateCustomAttribute(const CoreAttributes* ca,
                                       const QString name,
                                       QString& cellText) const
{
    // Handle custom attributes
    const CustomAttribute* custAttr =
        ca->getCustomAttribute(name);
    if (custAttr)
    {
        switch (custAttr->getType())
        {
            case CAT_Undefined:
                break;
            case CAT_Text:
                {
                    QString text =
                        dynamic_cast<const TextAttribute*>(custAttr)->
                        getText();
                    cellText = text;
                }
            case CAT_Reference:
                cellText =
                    dynamic_cast<const
                    ReferenceAttribute*>(custAttr)->getLabel();
                break;
        }
    }
}

void
TjPrintReport::computeTableMetrics()
{
    QFont fnt;
    QFontMetrics fm(fnt);

    /* We iterate over all the rows and determine their heights. This also
     * defines the top Y coordinate and the vertical page number. */
    int topOfRow = 0;
    int yPage = 0;
    for (QPtrListIterator<TjReportRow> rit(rows); *rit; ++rit)
    {
        (*rit)->setTopY(topOfRow);
        /* For each row we iterate over the cells to detemine their minimum
         * bounding rect. For each cell use the minimum width to determine the
         * overall column width. */
        int maxHeight = 0;
        for (int col = 0; col < getNumberOfColumns(); ++col)
        {
            TjReportCell* cell = (*rit)->getCell(col);
            QRect br = fm.boundingRect(cell->getText());
            if (br.height() > maxHeight)
                maxHeight = br.height();
            if (columns.at(col)->getWidth() < br.width())
                columns.at(col)->setWidth(br.width());
        }
        topOfRow += maxHeight;
        if (topOfRow > pageHeight)
        {
            topOfRow = 0;
            (*rit)->setTopY(0);
            yPage++;
        }
        (*rit)->setYPage(yPage);
    }

    /* Now that we know all the column widths, we can determine their absolute
     * X coordinate and the X page. */
    int colX = 0;
    int xPage = 0;
    for (QPtrListIterator<TjReportColumn> cit(columns); *cit; ++cit)
    {
        (*cit)->setLeftX(colX);
        colX += (*cit)->getWidth();
        if (colX > pageWidth)
        {
            colX = 0;
            (*cit)->setLeftX(0);
            xPage++;
        }
        (*cit)->setXPage(xPage);
    }
}

