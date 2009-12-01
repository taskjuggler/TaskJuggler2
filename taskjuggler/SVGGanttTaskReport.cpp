/*
 * SVGGanttTaskReport.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <config.h>

#ifdef HAVE_KDE

#include "SVGGanttTaskReport.h"

#include <math.h>
#include <qptrdict.h>
#include <qfile.h>
#include <klocale.h>

#include "tjlib-internal.h"
#include "Project.h"
#include "Utility.h"
#include "ExpressionTree.h"
#include "Task.h"
#include "Resource.h"
#include "Operation.h"
#include "Scenario.h"
#include "TableLineInfo.h"

// Older versions of KDE do not have this macro
#ifndef KDE_IS_VERSION
#define KDE_IS_VERSION(a,b,c) 0
#endif

#define DATE_TO_X(A) margex + ((long)((((double)(A)-(double)datemin)*width)/((double)datemax-(double)datemin)))
#define DUR_TO_W(A) ((long)((double)(A)*width/(datemax-datemin)))
// The following macro assumes that b <= c
#define SETINLIMITS(a,b,c) ((a) < (b) ? (b) : (a) > (c) ? (c) : (a))
#define CALCULATEXCOORDINATES(x,w,x2) calculateXCoordinatesInt(x,w,x2,task,scenario,datemin,datemax,margex,width)

void calculateXCoordinatesInt(unsigned int *px, unsigned int *pw, unsigned int *pw2,
    const Task* task, const int scenario,
    const time_t datemin, const time_t datemax, const unsigned int margex, const unsigned int width)
{
    unsigned int w = 0;
    unsigned int w2 = 0;
    unsigned int ex = 0;
    unsigned int ex2 = 0;
    unsigned int dur = 0;

    ex = DATE_TO_X(SETINLIMITS(task->getStart(scenario),datemin,datemax));
    ex2 = DATE_TO_X(SETINLIMITS(task->getEnd(scenario),datemin,datemax));

    w = ex2 - ex;

    if (px) *px = ex;
    if (pw) *pw = w;
    if (pw2) {
        dur = (unsigned int)((task->getEnd(scenario)- task->getStart(scenario)) * (task->getCompletionDegree(scenario) / 100.0));
        if (task->getStart(scenario) < datemin)
            dur -= datemin - task->getStart(scenario);
        if (task->getEnd(scenario) > datemax)
            dur -= task->getEnd(scenario) - datemax;
        w2 = DUR_TO_W(dur);
        *pw2 = w2;
    }

}

SVGGanttTaskReport::SVGGanttTaskReport(Project* p, const QString& file, const QString& defFile,
                       int dl) :
    SVGReport(p, file, defFile, dl),
    ReportElementBase(this),
    hideLinks(0)
{
    // default headline
    setHeadline(getProject()->getName() + " Gantt.");

    taskSortCriteria[0] = CoreAttributesList::TreeMode;
}

bool
SVGGanttTaskReport::generate()
{


    const unsigned int fontHeight = 6;
    const unsigned int lh = (unsigned int)(fontHeight * 1.6);  // Line height
    const unsigned int margex = 120;
    const unsigned int margey = 10 * fontHeight;
    time_t datemin = 0, datemax = 0, date = 0;
    unsigned int nbMilestonesToDisplay = 0;
    QValueList<int>::const_iterator it;
    const long width = 1200;
    const int gap = 10; // Spacing between axes and text
    QDomElement svgText, diagonal, rect, polyline, circlemark, taskName, line ;
    QDomDocument doc;
    Task* task = 0;
    Task* task2 = 0;
    int scenario;
    QString label;

    if( !open())
    {
        tjWarning(i18n("Can not open File '%1' for writing!")
                 .arg(fileName));
        return false;
    }

    // Loop to reset internal calculated index
    for (it = scenarios.begin(); it != scenarios.end(); ++it)
    {
        scenario = *it;
        for (TaskListIterator tli(project->getTaskListIterator()); *tli != 0; ++tli)
        {
            task = *tli;
            task->setSvgGanttReportIndex(scenario, -1);
        }
    }

    TaskList filteredTaskList;
    if (!filterTaskList(filteredTaskList, 0, getHideTask(), 0))
        return false;
    sortTaskList(filteredTaskList);

    // Loop to get min and max dates
    for (it = scenarios.begin(); it != scenarios.end(); ++it)
    {
        scenario = *it;
        for (TaskListIterator tli(filteredTaskList); *tli != 0; ++tli)
        {
            task = *tli;
            ++nbMilestonesToDisplay;
            date = task->getStart(scenario);
            if (date > 0)
            {
                if (datemin == 0 || datemin > task->getStart(scenario))
                    datemin = task->getStart(scenario);
                if (datemax == 0 || datemax < task->getEnd(scenario))
                    datemax = task->getEnd(scenario);
            }
        }
    }

    // Limit to user defined dates
    if (datemin < getStart()) datemin = getStart();
    if (datemax > getEnd()) datemax = getEnd();

    // To avoid division by 0 :
    if (datemin == datemax)
    {
        datemin = beginOfMonth(datemin);
        datemax = sameTimeNextMonth(datemin);
    }

    unsigned int mindatex = DATE_TO_X(datemin);
    unsigned int maxdatex = DATE_TO_X(datemax);

    // Add xml stadard info
    doc.appendChild(doc.createProcessingInstruction
                     ("xml", "version=\"1.0\" encoding=\"UTF-8\""));

    // Add taskjuggler comments
    if (timeStamp)
    {
        doc.appendChild(doc.createComment(
            QString("This file has been generated by TaskJuggler. For details about TaskJuggler see ") + TJURL));
    }

    // Create svg node
    QDomElement svg = doc.createElement("svg");
    doc.appendChild(svg);

    // Add stavdart svg attributes
    svg.setAttribute("xmlns", "http://www.w3.org/2000/svg");
    svg.setAttribute("version", "1.1");
    svg.setAttribute("width", QString::number(DATE_TO_X(datemax) + gap + margex));
    svg.setAttribute("height", QString::number(margey + 2 * gap + filteredTaskList.count() * scenarios.count() * lh));

    // Create and add title node
    QDomElement title = doc.createElement("title");
    title.appendChild(doc.createTextNode(getHeadline()));
    svg.appendChild(title);

    // Create and add desc node
    QDomElement desc = doc.createElement("desc");
    desc.appendChild(doc.createTextNode("Gantt diagram."));
    svg.appendChild(desc);

    // Create headline text
    if (!getHeadline().isEmpty())
    {
        svgText = doc.createElement("text");
        svgText.appendChild(doc.createTextNode(getHeadline()));
        svg.appendChild(svgText);
        svgText.setAttribute("x", margex + width / 2);
        svgText.setAttribute("y", fontHeight * 2);
        svgText.setAttribute("text-anchor", "middle");
        svgText.setAttribute("fill", "black");
        svgText.setAttribute("font-family", "Courrier");
        svgText.setAttribute("font-size", fontHeight * 1.5);
    }

    // Create caption text
    if (!getCaption().isEmpty())
    {
        svgText = doc.createElement("text");
        svgText.appendChild(doc.createTextNode(getCaption()));
        svg.appendChild(svgText);
        svgText.setAttribute("x", margex);
        svgText.setAttribute("y", width + margey + getProject()->getTimeFormat().length() * fontHeight );
        svgText.setAttribute("fill", "black");
        svgText.setAttribute("font-family", "Courrier");
        svgText.setAttribute("font-size", fontHeight * 0.8);
        svgText.setAttribute("font-style", "italic");
    }

    unsigned int x = 0, y = 0, i = 0, w = 0, h = 0, w2 = 0;

    // Add legend on X and Y axes
    // First, check what unit is to be used.
    double pixelPerDay = double(width) / (datemax - datemin) * 3600 * 24;
    enum eAxeUnit { eDay, eWeek, eMonth, eQuarter, eYear };
    eAxeUnit axeUnit = eYear;
    if (pixelPerDay > 2 * fontHeight)
        axeUnit = eDay;
    else {
        pixelPerDay *= 7;
        if (pixelPerDay > 2 * fontHeight)
            axeUnit = eWeek;
        else {
            pixelPerDay *= 4;
            if (pixelPerDay > 2 * fontHeight)
                axeUnit = eMonth;
            else {
                pixelPerDay *= 3;
                if (pixelPerDay > 2 * fontHeight)
                    axeUnit = eQuarter;
                else {
                    axeUnit = eYear;
                }
            }
        }
    }

    for (time_t axeDate =
            axeUnit == eDay? sameTimeNextDay(midnight(datemin-1)):
            axeUnit == eWeek? sameTimeNextWeek(beginOfWeek(datemin-1, project->getWeekStartsMonday())):
            axeUnit == eMonth? sameTimeNextMonth(beginOfMonth(datemin-1)):
            axeUnit == eQuarter? sameTimeNextQuarter(beginOfQuarter(datemin-1)):
                sameTimeNextYear(beginOfYear(datemin-1));
        axeDate <= datemax + 1;
        axeDate =
            axeUnit == eDay? sameTimeNextDay(axeDate) :
            axeUnit == eWeek? sameTimeNextWeek(axeDate) :
            axeUnit == eMonth? sameTimeNextMonth(axeDate):
            axeUnit == eQuarter? sameTimeNextQuarter(axeDate):
                sameTimeNextYear(axeDate)
    )
    {
        // Add X axe legend
        svgText = doc.createElement("text");
        svgText.appendChild(doc.createTextNode(time2user(axeDate, getTimeFormat())));
        svg.appendChild(svgText);
        x = DATE_TO_X(axeDate);
        svgText.setAttribute("x", x);
        y = margey + gap / 2;
        svgText.setAttribute("y", y);
        svgText.setAttribute("text-anchor", "start");
        svgText.setAttribute("fill", "black");
        svgText.setAttribute("rotate", 45);
        QString rotate = QString("rotate(");
        rotate += QString::number(-45) + " " + QString::number(x) + " " + QString::number(y) + ")";
        svgText.setAttribute("transform", rotate);
        svgText.setAttribute("font-family", "Courrier");
        svgText.setAttribute("font-size", fontHeight);

        // Draw vertical line
        line = doc.createElement("line");
        svg.appendChild(line);
        y = margey + gap;
        line.setAttribute("x1", x);
        line.setAttribute("y1", y);
        line.setAttribute("x2", x);
        line.setAttribute("y2", y + filteredTaskList.count() * scenarios.count() * lh);
        line.setAttribute("stroke", "grey");
        line.setAttribute("stroke-width", 1);
    }

    // Draw red vertical line for now, if in the given period
    if (getProject()->getNow() >= datemin && getProject()->getNow() <= datemax)
    {
        line = doc.createElement("line");
        svg.appendChild(line);
        x = DATE_TO_X(getProject()->getNow());
        y = margey + gap;
        line.setAttribute("x1", x);
        line.setAttribute("y1", y);
        line.setAttribute("x2", x);
        line.setAttribute("y2", y + filteredTaskList.count() * scenarios.count() * lh);
        line.setAttribute("stroke", "red");
        line.setAttribute("stroke-width", 1);
    }

    x = 0, y = 0, i = 0, w = 0, h = 0, w2 = 0;

    for (TaskListIterator tli(filteredTaskList); *tli != 0; ++tli)
    {
        task = *tli;

        // Add comment, task name
        svg.appendChild(doc.createComment(task->getName()));

        for (it = scenarios.begin(); it != scenarios.end(); ++it, i++)
        {
            scenario = *it;

            // Store index for use below.
            task->setSvgGanttReportIndex(scenario, i);

            /* We only handle properly scheduled tasks. */
            if (task->getStart(scenario) == 0 || task->getEnd(scenario) == 0)
                continue;

            // Add comment, scenario name
            svg.appendChild(doc.createComment("Scenario : " + getProject()->getScenario(*it)->getName()));

//             x = SETINLIMITS((unsigned int)DATE_TO_X(task->getStart(scenario)), mindatex, maxdatex);
//             x = DATE_TO_X(SETINLIMITS(task->getStart(scenario),datemin,datemax));
//             CALCULATEXCOORDINATES(&x,&w,0);
            y = (unsigned int)(margey + (i + 1) * lh);

            bool hasError = false;
            if ((task->getMaxStart(scenario) != 0 && task->getStart(scenario) > task->getMaxStart(scenario))
            ||  (task->getMinStart(scenario) != 0 && task->getStart(scenario) < task->getMinStart(scenario) )
            ||  (task->getMaxEnd(scenario) != 0 && task->getEnd(scenario) > task->getMaxEnd(scenario))
            ||  (task->getMinEnd(scenario) != 0 && task->getEnd(scenario) < task->getMinEnd(scenario)))
            {
                hasError = true;
            }

            if (task->isMilestone())
            {
//                 if (task->getStart(scenario) < datemin || task->getStart(scenario) > datemax)
//                     continue;

                CALCULATEXCOORDINATES(&x,0,0);
                w = lh / 3;
                h = y + lh / 6;

                polyline = doc.createElement("polyline");
                svg.appendChild(polyline);
                polyline.setAttribute("points",
                    QString::number(x) + "," + QString::number(h) + " " +
                    QString::number(x+w) + "," + QString::number(h+w) + " " +
                    QString::number(x) + "," + QString::number(h+2*w) + " " +
                    QString::number(x-w) + "," + QString::number(h+w) + " " +
                    QString::number(x) + "," + QString::number(h)
                );
                polyline.setAttribute("fill", hasError? "Red" : "black");
                polyline.setAttribute("stroke", "none");

                // w is used below, so correctly set it to 0 now:
                w = 0;
            }
            else if (task->isLeaf())
            {
//                 w = DUR_TO_W(task->getEnd(scenario)- task->getStart(scenario));
//                 if (x + w > maxdatex) w = maxdatex - x;
                CALCULATEXCOORDINATES(&x,&w,&w2);
                h = lh * 3 / 4;

                rect = doc.createElement("rect");
                svg.appendChild(rect);
                rect.setAttribute("x", x);
                rect.setAttribute("y", y + h / 4);
                rect.setAttribute("width", w);
                rect.setAttribute("height", h);
                rect.setAttribute("stroke", "black");
                rect.setAttribute("fill", hasError? "Red" : "lightblue");
                rect.setAttribute("stroke-width", 1);

/*                !!w2 = DUR_TO_W((task->getEnd(scenario)- task->getStart(scenario)) * (task->getCompletionDegree(scenario) / 100.0));
                !!if (x + w2 > maxdatex) w2 = maxdatex - x;*/
                h = lh / 4;

                rect = doc.createElement("rect");
                svg.appendChild(rect);
                rect.setAttribute("x", x);
                rect.setAttribute("y", y + lh * 3 / 8);
                rect.setAttribute("width", w2);
                rect.setAttribute("height", h);
                rect.setAttribute("stroke", "none");
                rect.setAttribute("fill", "black");

            }
            else
            {
                h = lh / 4;
//                 w = DUR_TO_W(task->getEnd(scenario)- task->getStart(scenario));
//                 if (x + w > maxdatex) w = maxdatex - x;
                CALCULATEXCOORDINATES(&x,&w,0);

                polyline = doc.createElement("polyline");
                svg.appendChild(polyline);
                polyline.setAttribute("points",
                    QString::number(x) + "," + QString::number(y+3*h) + " " +
                    QString::number(x-h) + "," + QString::number(y+2*h) + " " +
                    QString::number(x-h) + "," + QString::number(y+h) + " " +
                    QString::number(x+w+h) + "," + QString::number(y+h) + " " +
                    QString::number(x+w+h) + "," + QString::number(y+2*h) + " " +
                    QString::number(x+w) + "," + QString::number(y+3*h) + " " +
                    QString::number(x+w-h) + "," + QString::number(y+2*h) + " " +
                    QString::number(x+h) + "," + QString::number(y+2*h) + " " +
                    QString::number(x) + "," + QString::number(y+3*h)
                );
                polyline.setAttribute("fill", hasError? "Red" : "black");
                polyline.setAttribute("stroke", "none");
            }

            if (taskBarPrefix != "" || taskBarPostfix != "")
            {
                TableLineInfo tli;
                tli.sc = scenario;
                tli.ca1 = task;
                tli.task = task;
                setMacros(&tli);
            }
            if ((label = expandReportVariable(taskBarPrefix)) != "")
            {
                svgText = doc.createElement("text");
                svgText.appendChild(doc.createTextNode(label));
                svg.appendChild(svgText);
                svgText.setAttribute("x", x - gap);
                svgText.setAttribute("y", y + fontHeight);
                svgText.setAttribute("text-anchor", "end");
                svgText.setAttribute("fill", "black");
                svgText.setAttribute("font-family", "Courrier");
                svgText.setAttribute("font-size", fontHeight);
            }
            if ((label = expandReportVariable(taskBarPostfix)) != "")
            {
                svgText = doc.createElement("text");
                svgText.appendChild(doc.createTextNode(label));
                svg.appendChild(svgText);
                svgText.setAttribute("x", x + w + gap);
                svgText.setAttribute("y", y + fontHeight);
                svgText.setAttribute("text-anchor", "start");
                svgText.setAttribute("fill", "black");
                svgText.setAttribute("font-family", "Courrier");
                svgText.setAttribute("font-size", fontHeight);
            }
        }
    }

    unsigned int x2 = 0, y2 = 0;
    x = 0; y = 0; i = 0;w = 0; h = 0; w2 = 0;
    if (!getHideLinks())
    {
        for (TaskListIterator tli(filteredTaskList); *tli != 0; ++tli)
        {
            task = *tli;

            // Add comment, task name
            svg.appendChild(doc.createComment(task->getName()));

            for (it = scenarios.begin(); it != scenarios.end(); ++it, i++)
            {
                scenario = *it;

                /* We only handle properly scheduled tasks. */
                if (task->getStart(scenario) == 0 || task->getEnd(scenario) == 0 || task->getSvgGanttReportIndex(scenario) < 0)
                    continue;

/*                x = SETINLIMITS((unsigned int)DATE_TO_X(task->getStart(scenario)), mindatex, maxdatex);
                w = DUR_TO_W(task->getEnd(scenario)- task->getStart(scenario));*/
                y = (unsigned int)(margey + (i + 1) * lh);
                CALCULATEXCOORDINATES(&x,&w,0);
                if (x + w > maxdatex) w = maxdatex - x;

                for (TaskListIterator tli2(task->getFollowersIterator()); *tli2; ++tli2)
                {
                    task2 = *tli2;

                    /* We only handle properly scheduled tasks. */
                    if (task2->getStart(scenario) == 0 || task2->getEnd(scenario) == 0 || task2->getSvgGanttReportIndex(scenario) < 0)
                        continue;

                    /* If the parent has the same follower, it's an inherited dependency
                    * and we don't have to draw the arrows for every sub task. */
                    if (task2->getParent() && task2->getParent()->hasPrevious(task))
                        continue;

                    x2 = SETINLIMITS((unsigned int)DATE_TO_X(task2->getStart(*it)), mindatex, maxdatex);
                    y2 = (unsigned int)(margey + (task2->getSvgGanttReportIndex(scenario) + 1) * lh);

                    h = ((int)y2 - (int)y) < 0 ? y - y2 : y2 - y;

                    // The h value used for the svg quadratic calculation must be limited otherwise firefox and inkscape
                    // display strange lines or no line.
                    if (h > 120) h = 120;

                    // <path id="courbe" d="M0 0 C300,0 -100,300 200,300" stroke="blue" fill="none"/>
                    polyline = doc.createElement("path");
                    svg.appendChild(polyline);
                    polyline.setAttribute("d", "M" +
                        QString::number(x + w) + " " + QString::number(y + lh / 2) +
                        "C" + QString::number(x + w + h) + " " + QString::number(y + lh / 2) +
                        "," + QString::number(x2 - h) + " " + QString::number(y2 + lh / 2) +
                        "," + QString::number(x2) + " " + QString::number(y2 + lh / 2)
                    );
                    polyline.setAttribute("stroke", "darkorange");
                    polyline.setAttribute("fill", "none");
                }
            }
        }
    }

    // Write xml string to out file.
    s << doc.toString();

    // Close file
    return close();
}

void SVGGanttTaskReport::inheritValues()
{
    SVGReport::inheritValues();

    SVGGanttTaskReport* parent = dynamic_cast<SVGGanttTaskReport*>(getParentReport());

    if (parent)
    {
        setHideLinks(parent->getHideLinks());
        setTaskBarPrefix(parent->getTaskBarPrefix());
        setTaskBarPostfix(parent->getTaskBarPostfix());
    }
}

#endif
