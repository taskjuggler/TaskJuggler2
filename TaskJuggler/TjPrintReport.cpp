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

#include <config.h>
#include <assert.h>

#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qfontmetrics.h>

#include <klocale.h>

#include "Report.h"
#include "QtReportElement.h"
#include "QtReport.h"
#include "TableColumnInfo.h"
#include "TableColumnFormat.h"
#include "ReportElement.h"
#include "Project.h"
#include "Task.h"
#include "Resource.h"
#include "TjReportCell.h"
#include "TjReportRow.h"
#include "TjReportColumn.h"
#include "TextAttribute.h"
#include "ReferenceAttribute.h"
#include "TjGanttChart.h"

TjPrintReport::TjPrintReport(const Report* rd, QPaintDevice* pd) :
    reportDef(rd), paintDevice(pd)
{
    rows.setAutoDelete(TRUE);
    columns.setAutoDelete(TRUE);

    showGantt = TRUE;
    if (showGantt)
    {
        ganttChartObj = new QObject();
        ganttChart = new TjGanttChart(ganttChartObj);
    }

    leftMargin = topMargin = pageWidth = pageHeight = 0;
    cellMargin = 0;
    headlineHeight = 0;
    tableRight = tableBottom = 0;

    standardFont.setPixelSize(pointsToYPixels(7));
    tableHeaderFont.setPixelSize(pointsToYPixels(7));
    headlineFont.setPixelSize(pointsToYPixels(12));
    signatureFont.setPixelSize(pointsToYPixels(4));

    indentSteps = mmToXPixels(3);
}

TjPrintReport::~TjPrintReport()
{
}

void
TjPrintReport::getNumberOfPages(int& xPages, int& yPages)
{
    assert(columns.last() != 0);
    xPages = columns.last()->getXPage() + 1;
    assert(rows.last() != 0);
    yPages = rows.last()->getYPage() + 1;
}

void
TjPrintReport::generateTableHeader()
{
    for (QPtrListIterator<TableColumnInfo>
         ci = reportElement->getColumnsIterator(); *ci; ++ci)
    {
        TjReportColumn* col = new TjReportColumn;
        const TableColumnFormat* tcf =
            reportElement->getColumnFormat((*ci)->getName());
        col->setTableColumnFormat(tcf);
        if (tcf->getIndent())
        {
            int maxIndentLevel = 1;
            if (((tcf->genTaskLine1 &&
                  tcf->genTaskLine1 != &ReportElement::genCellEmpty) &&
                 tcf->genResourceLine2) ||
                ((tcf->genResourceLine1 &&
                  tcf->genResourceLine1 != &ReportElement::genCellEmpty) &&
                 tcf->genTaskLine2))
                maxIndentLevel = maxDepthTaskList + maxDepthResourceList;
            else if ((tcf->genTaskLine1 &&
                      tcf->genTaskLine1 != &ReportElement::genCellEmpty) &&
                     !tcf->genResourceLine2)
                maxIndentLevel = maxDepthTaskList;
            else if ((tcf->genResourceLine1 &&
                      tcf->genResourceLine1 != &ReportElement::genCellEmpty) &&
                     !tcf->genTaskLine2)
                maxIndentLevel = maxDepthResourceList;
            col->setMaxIndentLevel(maxIndentLevel);
        }

        columns.append(col);
    }

    if (showGantt)
    {
        TjReportColumn* col = new TjReportColumn;
        col->setIsGantt(TRUE);
        columns.append(col);
    }
}

void
TjPrintReport::generateTaskListRow(TjReportRow* row, const Task* task,
                                   const Resource* resource)
{
    int colIdx= 0;
    for (QPtrListIterator<TableColumnInfo>
         ci = reportElement->getColumnsIterator(); *ci; ++ci, ++colIdx)
    {
        QString cellText;
        TjReportCell* cell = new TjReportCell(row, columns.at(colIdx));
        const TableColumnFormat* tcf =
            reportElement->getColumnFormat((*ci)->getName());

        /* Determine whether the cell content should be indented. And if so,
         * then on what level. */
        if (tcf->getIndent() &&
            reportDef->getTaskSorting(0) == CoreAttributesList::TreeMode)
        {
            cell->setIndentLevel(task->treeLevel() -
                                 reportElement->taskRootLevel());
        }

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
        else if ((*ci)->getName() == "index")
            cellText.sprintf("%d.", task->getIndex());
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
        else if ((*ci)->getName() == "name")
        {
            if (reportDef->getTaskSorting(0) == CoreAttributesList::TreeMode)
            {
                if (resource)
                    cell->setIndentLevel(maxDepthResourceList +
                                         task->treeLevel() -
                                         reportElement->taskRootLevel());
                else
                    cell->setIndentLevel(task->treeLevel() -
                                         reportElement->taskRootLevel());
            }
            cellText = task->getName();
        }
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

        cell->setText(cellText);
        row->insertCell(cell, colIdx);
    }
}

void
TjPrintReport::generateResourceListRow(TjReportRow* row,
                                       const Resource* /*resource*/,
                                       const Task* /*task*/)
{
    int colIdx= 0;
    for (QPtrListIterator<TableColumnInfo>
         ci = reportElement->getColumnsIterator(); *ci; ++ci, ++colIdx)
    {
        QString cellText;
        TjReportCell* cell = new TjReportCell(row, columns.at(colIdx));
        cell->setText(cellText);
        row->insertCell(cell, colIdx);
    }
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
TjPrintReport::layoutPages(QPrinter::Orientation orientation)
{
    // Set the left and top margin to 2cm
    QPaintDeviceMetrics metrics(paintDevice);
    leftMargin = mmToXPixels(10);
    topMargin = mmToYPixels(10);
    if (orientation == QPrinter::Portrait)
    {
        pageWidth = metrics.width() - 2 * leftMargin;
        pageHeight = metrics.height() - 2 * topMargin;
    }
    else
    {
        pageWidth = metrics.height() - 2 * topMargin;
        pageHeight = metrics.width() - 2 * leftMargin;
    }

    cellMargin = mmToXPixels(1);

    // Determine height of headline
    if (!reportElement->getHeadline().isEmpty())
    {
        QFontMetrics fm(headlineFont);
        QRect br = fm.boundingRect(reportElement->getHeadline());
        int margin = mmToYPixels(2);
        headlineHeight = br.height() + 2 * margin + 1;
        headlineX = leftMargin + (pageWidth - br.width()) / 2;
        headlineBase = topMargin + headlineHeight - 1 - margin - fm.descent();
    }

    // Determine geometries for table header elements
    headerY = topMargin + headlineHeight;
    headerHeight = 0;
    for (QPtrListIterator<TjReportColumn> cit(columns); *cit; ++cit)
    {
        if (!(*cit)->getIsGantt())
        {
            const TableColumnFormat* tcf = (*cit)->getTableColumnFormat();
            QFontMetrics fm(tableHeaderFont);
            QRect br = fm.boundingRect(tcf->getTitle());
            br.setWidth(br.width() + 2 * cellMargin + 1);
            br.setHeight(br.height() + 2 * cellMargin + 1);
            if (br.height() > headerHeight)
                headerHeight = br.height();
            if ((*cit)->getWidth() < br.width())
                (*cit)->setWidth(br.width());
        }
        else
            headerHeight *= 2;
    }

    // Determine height of bottom line
    QFontMetrics fm(standardFont);
    bottomlineHeight = fm.height() + 2 * cellMargin;
    bottomlineY = topMargin + pageHeight - bottomlineHeight;

    // Determine the geometry of the footer
    footerHeight = mmToYPixels(15); // Use a fixed footer for now
    footerY = bottomlineY - footerHeight;

    // And now the table layout
    tableRight = leftMargin + pageWidth;
    tableBottom = bottomlineY - 1;

    /* We iterate over all the rows and determine their heights. This also
     * defines the top Y coordinate and the vertical page number. */
    int topOfRow = topMargin + headlineHeight + headerHeight;
    int yPage = 0;
    for (QPtrListIterator<TjReportRow> rit(rows); *rit; ++rit)
    {
        /* For each row we iterate over the cells to detemine their minimum
         * bounding rect. For each cell use the minimum width to determine the
         * overall column width. */
        int maxHeight = fm.height() + 2 * cellMargin;
        for (int col = 0; col < getNumberOfColumns(); ++col)
        {
            if (columns.at(col)->getIsGantt())
                continue;
            TjReportCell* cell = (*rit)->getCell(col);
            assert(cell != 0);
            assert(cell->getRow() == *rit);
            assert(cell->getColumn() == columns.at(col));
            if (cell->getText().isEmpty())
                continue;
            QRect br = fm.boundingRect(cell->getText());
            // Compute the indentation depth for the cell.
            int indentation = 0;
            if (columns.at(col)->getTableColumnFormat()->getHAlign() ==
                TableColumnFormat::left)
                indentation = cell->getIndentLevel() * indentSteps;
            else if (columns.at(col)->getTableColumnFormat()->getHAlign() ==
                     TableColumnFormat::right)
                indentation = (columns.at(col)->getMaxIndentLevel() - 1 -
                    cell->getIndentLevel()) * indentSteps;
            br.setWidth(br.width() + 2 * cellMargin + 1 + indentation);
            br.setHeight(br.height() + 2 * cellMargin + 1);
            if (br.height() > maxHeight)
                maxHeight = br.height();
            if (columns.at(col)->getWidth() < br.width())
                columns.at(col)->setWidth(br.width());
        }
        if (topOfRow + maxHeight > tableBottom)
        {
            topOfRow = topMargin + headlineHeight + headerHeight;
            yPage++;
        }
        (*rit)->setTopY(topOfRow);
        topOfRow += maxHeight;
        (*rit)->setYPage(yPage);
        (*rit)->setHeight(maxHeight);
    }

    /* Now that we know all the column widths, we can determine their absolute
     * X coordinate and the X page. */
    int colX = leftMargin;
    int xPage = 0;
    int ganttChartWidth = 0;
    for (QPtrListIterator<TjReportColumn> cit(columns); *cit; ++cit)
    {
        if (!(*cit)->getIsGantt())
        {
            if (colX + (*cit)->getWidth() > leftMargin + pageWidth)
            {
                // The first column is repeated at the left of each page
                colX = leftMargin + columns.at(0)->getWidth();
                xPage++;
            }
            (*cit)->setLeftX(colX);
            (*cit)->setXPage(xPage);
            colX += (*cit)->getWidth();
        }
        else
        {
            /* The Gantt chart should be at least 1/3 of the page width. If it
             * does not fit on this page anymore, we start a new page. */
            if (colX > leftMargin + (int) ((2.0 / 3) * pageWidth))
            {
                // The first column is repeated at the left of each page
                colX = leftMargin + columns.at(0)->getWidth();
                xPage++;
            }
            (*cit)->setLeftX(colX);
            (*cit)->setXPage(xPage);
            ganttChartWidth = leftMargin + pageWidth - colX;
            (*cit)->setWidth(ganttChartWidth);
            colX += (*cit)->getWidth();
        }
    }

    if (showGantt)
    {
        int tableHeight = 0;
        for (QPtrListIterator<TjReportRow> rit(rows); *rit; ++rit)
            tableHeight += (*rit)->getHeight();
        ganttChart->setProjectAndReportData(reportElement, &taskList,
                                            &resourceList);
        ganttChart->setSizes(headerHeight, tableHeight, ganttChartWidth);
        ganttChart->setColors(Qt::white, Qt::white, Qt::lightGray, Qt::gray,
                              Qt::lightGray, Qt::darkGray);
        ganttChart->setScaleMode(TjGanttChart::fitSize);
        ganttChart->generate();
    }
}

bool
TjPrintReport::beginPrinting()
{
    return p.begin(paintDevice);
}

void
TjPrintReport::endPrinting()
{
    p.end();
}

void
TjPrintReport::printReportPage(int x, int y)
{
    QFont fnt;
    QFontMetrics fm(fnt);
    int descent = fm.descent();

    // Draw outer frame
    p.setPen(QPen(Qt::black, 1));
    p.drawRect(leftMargin, topMargin, pageWidth, pageHeight);

    // Draw headline if needed
    if (headlineHeight > 0)
    {
        p.setFont(headlineFont);
        p.drawText(headlineX, headlineBase, reportElement->getHeadline());
        p.setPen(QPen(Qt::black, 1));
        p.drawLine(leftMargin, topMargin + headlineHeight - 1,
                   leftMargin + pageWidth, topMargin + headlineHeight - 1);
    }

    // Draw the table header
    for (QPtrListIterator<TjReportColumn> cit(columns); *cit; ++cit)
    {
        p.setPen(QPen(Qt::black, 1));
        p.setFont(tableHeaderFont);
        if ((*cit)->getIsGantt() && (*cit)->getXPage() == x)
        {
            QRect vp = p.viewport();
            vp.setX((*cit)->getLeftX());
            vp.setY(headerY);
            p.setViewport(vp);
            ganttChart->paintHeader(QRect(0, 0, (*cit)->getWidth(),
                                          headerHeight), &p);
            vp.setX(0);
            vp.setY(0);
            p.setViewport(vp);
        }
        else if ((*cit == columns.first() || (*cit)->getXPage() == x) &&
                 !(*cit)->getIsGantt())
        {
            const TableColumnFormat* tcf = (*cit)->getTableColumnFormat();
            p.drawText((*cit)->getLeftX() + cellMargin - 1,
                       headerY + headerHeight - cellMargin - descent - 1,
                       tcf->getTitle());
            // Draw right cell border
            p.drawLine((*cit)->getLeftX() + (*cit)->getWidth() - 1,
                       headerY, (*cit)->getLeftX() + (*cit)->getWidth() - 1,
                       headerY + headerHeight - 1);
        }
    }
    // Draw lower border of header
    p.drawLine(leftMargin, headerY + headerHeight - 1,
               leftMargin + pageWidth, headerY + headerHeight - 1);

    // Draw the table cells for this page
    bool ganttChartPainted = FALSE;
    for (QPtrListIterator<TjReportRow> rit(rows); *rit; ++rit)
        if ((*rit)->getYPage() == y)
        {
            for (int col = 0; col < getNumberOfColumns(); ++col)
            {
                TjReportColumn* reportColumn = columns.at(col);
                /* The first column is repeated as left column on each page.
                 * On the first page we of course don't have to do this. */
                if ((reportColumn->getXPage() == x || col == 0) &&
                    !reportColumn->getIsGantt())
                {
                    printReportCell(*rit, col);

                    // Draw lower border line for row
                    if (col == 0)
                    {
                        p.setPen(QPen(Qt::black, 1));
                        p.drawLine(leftMargin,
                                   (*rit)->getTopY() + (*rit)->getHeight() - 1,
                                   leftMargin + columns.at(0)->getWidth() - 1,
                                   (*rit)->getTopY() + (*rit)->getHeight() - 1);
                    }
                    else
                    {
                        p.setPen(QPen(Qt::gray, 1));
                        p.drawLine(reportColumn->getLeftX(),
                                   (*rit)->getTopY() + (*rit)->getHeight() - 1,
                                   reportColumn->getLeftX() +
                                   reportColumn->getWidth() - 1,
                                   (*rit)->getTopY() + (*rit)->getHeight() - 1);

                    }

                    // Draw right cell border
                    p.setPen(QPen(col == 0 ? Qt::black : Qt::gray, 1));
                    p.drawLine(columns.at(col)->getLeftX() +
                               columns.at(col)->getWidth() - 1,
                               (*rit)->getTopY(),
                               columns.at(col)->getLeftX() +
                               columns.at(col)->getWidth() - 1,
                               (*rit)->getTopY() + (*rit)->getHeight());
                }
                else if (reportColumn->getIsGantt() &&
                         reportColumn->getXPage() == x && !ganttChartPainted)
                {
                    ganttChartPainted = true;
                    QRect vp = p.viewport();
                    vp.setX(columns.at(col)->getLeftX());
                    vp.setY((*rit)->getTopY());
                    p.setViewport(vp);
                    int tableHeight = 0;
                    int prevTablesHeight = 0;
                    for (QPtrListIterator<TjReportRow> rit(rows); *rit; ++rit)
                        if ((*rit)->getYPage() < y)
                            prevTablesHeight += (*rit)->getHeight();
                        else if ((*rit)->getYPage() == y)
                            tableHeight += (*rit)->getHeight();
                    ganttChart->paintChart
                        (QRect(0, prevTablesHeight, columns.at(col)->getWidth(),
                               tableHeight), &p);
                    vp.setX(0);
                    vp.setY(0);
                    p.setViewport(vp);
                }
            }
        }

    // Draw footer
    p.setPen(QPen(Qt::black, 1));
    p.drawLine(leftMargin, footerY, leftMargin + pageWidth - 1, footerY);

    // Draw bottom line
    p.setPen(QPen(Qt::black, 1));
    p.drawLine(leftMargin, bottomlineY, leftMargin + pageWidth - 1,
               bottomlineY);
    // Page number in the center
    QString pageMark = i18n("Page %1 of %2")
        .arg(y * (columns.last()->getXPage() + 1) + x + 1)
        .arg((columns.last()->getXPage() + 1) * (rows.last()->getYPage() + 1));
    QFontMetrics fm1(standardFont);
    QRect br = fm1.boundingRect(pageMark);
    p.drawText(leftMargin + (pageWidth - br.width()) / 2,
               bottomlineY + cellMargin + fm1.ascent(), pageMark);
    // Copyright message on the left
    if (!reportDef->getProject()->getCopyright().isEmpty())
        p.drawText(leftMargin + cellMargin,
                   bottomlineY + cellMargin + fm1.ascent(),
                   reportDef->getProject()->getCopyright());
    // Project scheduding reference date (now value) on the right
    QString now = time2user(reportDef->getProject()->getNow(),
                            reportElement->getTimeFormat());
    br = fm1.boundingRect(now);
    p.drawText(leftMargin + pageWidth - cellMargin - br.width(),
               bottomlineY + cellMargin + fm1.ascent(), now);

    // Print a small signature below the page frame
    if (reportDef->getTimeStamp())
    {
        p.setFont(signatureFont);
        QFontMetrics fm(signatureFont);
        // The signature is not marked for translation on purpose!
        QString signature = QString("Generated on %1 with TaskJuggler %2")
            .arg(time2user(time(0), reportElement->getTimeFormat()))
            .arg(VERSION);
        QRect br = fm.boundingRect(signature);
        p.drawText(leftMargin + pageWidth - br.width(),
                   topMargin + pageHeight + cellMargin + fm.height(),
                   signature);
    }
}

void
TjPrintReport::printReportCell(TjReportRow* row, int col)
{
    QFontMetrics fm(standardFont);
    int descent = fm.descent();

    TjReportCell* cell = row->getCell(col);
    TjReportColumn* column = cell->getColumn();
    p.setFont(standardFont);
    p.setPen(QPen(Qt::black, 1));
    int y = row->getTopY() + row->getHeight() - cellMargin - descent - 1;
    switch (column->getTableColumnFormat()->getHAlign())
    {
        case TableColumnFormat::left:
            p.drawText(column->getLeftX() + cellMargin - 1 +
                       cell->getIndentLevel() * indentSteps, y,
                       cell->getText());
            break;
        case TableColumnFormat::center:
        {
            QFontMetrics fm(standardFont);
            QRect br = fm.boundingRect(cell->getText());
            int x = (columns.at(col)->getWidth() -
                     (2 * cellMargin + br.width())) / 2;
            p.drawText(column->getLeftX() + cellMargin - 1 + x, y,
                       cell->getText());
            break;
        }
        case TableColumnFormat::right:
        {
            QFontMetrics fm(standardFont);
            QRect br = fm.boundingRect(cell->getText());
            int x = columns.at(col)->getWidth() -
                (2 * cellMargin + br.width() +
                 ((column->getMaxIndentLevel() - 1 -
                  cell->getIndentLevel()) * indentSteps));
            p.drawText(column->getLeftX() + cellMargin - 1 + x, y,
                       cell->getText());
        }
    }
}

int
TjPrintReport::mmToXPixels(int mm)
{
    QPaintDeviceMetrics metrics(paintDevice);
    return (int) ((mm / 25.4) * metrics.logicalDpiX());
}

int
TjPrintReport::mmToYPixels(int mm)
{
    QPaintDeviceMetrics metrics(paintDevice);
    return (int) ((mm / 25.4) * metrics.logicalDpiY());
}

int
TjPrintReport::pointsToYPixels(int pts)
{
    QPaintDeviceMetrics metrics(paintDevice);
    return (int) ((pts * (0.376 / 25.4)) * metrics.logicalDpiY());
}

