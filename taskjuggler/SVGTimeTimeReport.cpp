/*
 * SVGTimeTimeReport.cpp - TaskJuggler
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

#include "SVGTimeTimeReport.h"

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

// Older versions of KDE do not have this macro
#ifndef KDE_IS_VERSION
#define KDE_IS_VERSION(a,b,c) 0
#endif

// Here is the SVG color list. Too light colors are commented out.
const char* svgColors[] = {
//     "aliceblue", /*Removed, too light*/
//     "antiquewhite", /*Removed, too light*/
    "aquamarine",
    "aqua",
//     "azure", /*Removed, too light*/
//     "beige", /*Removed, too light*/
    "bisque",
    "black",
//     "blanchedalmond", /*Removed, too light*/
    "blue",
    "blueviolet",
    "brown",
    "burlywood",
    "cadetblue",
    "chartreuse",
    "chocolate",
    "coral",
    "cornflowerblue",
//     "cornsilk", /*Removed, too light*/
    "crimson",
    "cyan",
    "darkblue",
    "darkcyan",
    "darkgoldenrod",
    "darkgray",
    "darkgreen",
    "darkgrey",
    "darkkhaki",
    "darkmagenta",
    "darkolivegreen",
    "darkorange",
    "darkorchid",
    "darkred",
    "darksalmon",
    "darkseagreen",
    "darkslateblue",
    "darkslategray",
    "darkslategrey",
    "darkturquoise",
    "darkviolet",
    "deeppink",
    "deepskyblue",/* Removed, too light*/
    "dimgray",
    "dimgrey",
    "dodgerblue",
    "firebrick",
//     "floralwhite",
    "forestgreen",
    "fuchsia",
    "gainsboro",
//     "ghostwhite",
    "gold",
    "goldenrod",
    "gray",
    "green",
    "greenyellow",
    "grey",
//     "honeydew",
    "hotpink",
    "indianred",
    "indigo",
//     "ivory",
    "khaki",
    "lavender",
//     "lavenderblush",
    "lawngreen",
//     "lemonchiffon",
    "lightblue",
    "lightcoral",
//     "lightcyan",
//     "lightgoldenrodyellow",
    "lightgray",
    "lightgreen",
    "lightgrey",
    "lightpink",
    "lightsalmon",
    "lightseagreen",
    "lightskyblue",
    "lightslategray",
    "lightslategrey",
    "lightsteelblue",
//     "lightyellow",
    "limegreen",
    "lime",
//     "linen",
    "magenta",
    "maroon",
    "mediumaquamarine",
    "mediumblue",
    "mediumorchid",
    "mediumpurple",
    "mediumseagreen",
    "mediumslateblue",
    "mediumspringgreen",
    "mediumturquoise",
    "mediumvioletred",
    "midnightblue",
//     "mintcream",
//     "mistyrose",
//     "moccasin",
    "navajowhite",
    "navy",
//     "oldlace",
    "olive",
    "olivedrab",
    "orange",
    "orangered",
    "orchid",
    "palegoldenrod",
    "palegreen",
    "paleturquoise",
    "palevioletred",
    "papayawhip",
    "peachpuff",
    "peru",
    "pink",
    "plum",
    "powderblue",
    "purple",
    "red",
    "rosybrown",
    "royalblue",
    "saddlebrown",
    "salmon",
    "sandybrown",
    "seagreen",
//     "seashell",
    "sienna",
    "silver",
    "skyblue",
    "slateblue",
    "slategray",
    "slategrey",
//     "snow",
    "springgreen",
    "steelblue",
    "tan",
    "teal",
    "thistle",
    "tomato",
    "turquoise",
    "violet",
    "wheat",
//     "white",
//     "whitesmoke",
    "yellowgreen",
    "yellow"
};


SVGTimeTimeReport::SVGTimeTimeReport(Project* p, const QString& file, const QString& defFile,
                       int dl) :
    SVGReport(p, file, defFile, dl)
{
    // default headline
    setHeadline(getProject()->getName() + " time/time report.");
}

bool
SVGTimeTimeReport::generate()
{

#define DATE_TO_X(A) margex + ((long)((((double)(A)-(double)datemin)*width)/((double)datemax-(double)datemin)))
#define DATE_TO_Y(A) margey + (width - (long)((((double)(A)-(double)datemin)*width)/((double)datemax-(double)datemin)))

    const int fontHeight = 10;
    const int margex = 120;
    const int margey = 3 * fontHeight;
    time_t datemin = 0, datemax = 0, date = 0;
    unsigned int nbMilestonesToDisplay = 0;
    QValueList<int>::const_iterator it;
    const long width = 1000;
    const int gap = 10; // Spacing between axes and text
    QDomElement svgText, diagonal, rect, polyline, circlemark, milestonedate, taskName, line ;
    QDomDocument doc;

    if( !open())
    {
        tjWarning(i18n("Can not open File '%1' for writing!")
                 .arg(fileName));
        return false;
    }

    TaskList filteredTaskList;
    if (!filterTaskList(filteredTaskList, 0, getHideTask(), 0))
        return false;
    sortTaskList(filteredTaskList);

    // Loop to get min and max dates
    for (it = scenarios.begin(); it != scenarios.end(); ++it)
    {
        date = getProject()->getScenario(*it)->getDate();
        if (date > 0)
        {
            if (datemin == 0 || datemin > date)
                datemin = date;
            if (datemax == 0 || datemax < date)
                datemax = date;
        }
        for (TaskListIterator tli(filteredTaskList); *tli != 0; ++tli)
        {
            ++nbMilestonesToDisplay;
            date = (*tli)->getStart(*it);
            if (date > 0)
            {
                if (datemin == 0 || datemin > date)
                    datemin = date;
                if (datemax == 0 || datemax < date)
                    datemax = date;
            }
        }
    }

    // To avoid division by 0 :
    if (datemin == datemax)
    {
        datemin = beginOfMonth(datemin);
        datemax = sameTimeNextMonth(datemin);
    }

    // Now we know min and max, and convert these to reasonnable x,y values.
    long xmin = DATE_TO_X(datemin);
    long xmax = DATE_TO_X(datemax);
    long ymin = DATE_TO_Y(datemin);
    long ymax = DATE_TO_Y(datemax);

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

    // Add javascript code
    QDomElement script = doc.createElement("script");
    svg.appendChild(script);
    script.setAttribute("type", "text/ecmascript");
    script.appendChild(doc.createCDATASection(
        "    function jmouseover(evt, object) {"
        "        evt.stopPropagation();"
        "        document.getElementById('polyline.' + object).setAttributeNS(null,'stroke-width', 5);"
        "        document.getElementById('legendline.' + object).setAttributeNS(null,'stroke-width', 5);"
        "        document.getElementById('legendtext.' + object).setAttributeNS(null,'font-style', 'italic');"
        "    }"
        "    function jmouseout(evt, object) {"
        "        evt.stopPropagation();"
        "        document.getElementById('polyline.' + object).setAttributeNS(null,'stroke-width', 1);"
        "        document.getElementById('legendline.' + object).setAttributeNS(null,'stroke-width', 1);"
        "        document.getElementById('legendtext.' + object).setAttributeNS(null,'font-style', 'normal');"
        "    }"
        ));

    // Create and add title node
    QDomElement title = doc.createElement("title");
    title.appendChild(doc.createTextNode(getHeadline()));
    svg.appendChild(title);

    // Create and add desc node
    QDomElement desc = doc.createElement("desc");
    desc.appendChild(doc.createTextNode("Time-time milestone diagram."));
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

    // Create graphic rectangle area
    rect = doc.createElement("rect");
    svg.appendChild(rect);
    rect.setAttribute("width", width);
    rect.setAttribute("height", width);
    rect.setAttribute("x", xmin);
    rect.setAttribute("y", ymax);
    rect.setAttribute("stroke", "black");
    rect.setAttribute("fill", "none");
    rect.setAttribute("stroke-width", 1);

    // Create diagonal line.
    diagonal = doc.createElement("line");
    svg.appendChild(diagonal);
    diagonal.setAttribute("x1", xmin);
    diagonal.setAttribute("y1", ymin);
    diagonal.setAttribute("x2", xmax);
    diagonal.setAttribute("y2", ymax);
    diagonal.setAttribute("stroke", "black");
    diagonal.setAttribute("stroke-width", 1);

    // First, loop on tasks, to get longest task name
    unsigned int maxTaskNameLength = 0;
    for (TaskListIterator tli(filteredTaskList); *tli != 0; ++tli)
    {
        if ((*tli)->getName().length() > maxTaskNameLength)
        {
            maxTaskNameLength = (*tli)->getName().length();
        }
    }

    unsigned int colorSelection = 0, i = 0;
    unsigned int taskLegendX = 0, taskLegendY = 0;
    time_t x = 0, y = 0;
    QString polylinepoints;
    unsigned int nbLegendPerColumn = (width + getProject()->getTimeFormat().length() * fontHeight) / fontHeight / 2;

    svg.setAttribute("width", margex + width + 2 * gap + ( 1 + filteredTaskList.count() / nbLegendPerColumn) * maxTaskNameLength * fontHeight);
    svg.setAttribute("height", margey + width + 2 * gap + 10 * fontHeight);

    for (TaskListIterator tli(filteredTaskList); *tli != 0; ++tli, i++)
    {
        // Add comment, task name
        svg.appendChild(doc.createComment("Task : " + (*tli)->getName()));

        colorSelection = i %  (sizeof(svgColors) / sizeof(char*));

        polylinepoints = "";

        // Add task legend
        taskLegendX = width + margex + 2 * gap + (i / nbLegendPerColumn) * maxTaskNameLength * fontHeight;
        taskLegendY = margey + i % nbLegendPerColumn * fontHeight * 2;

        // Add legend first circle mark
        line = doc.createElement("line");
        svg.appendChild(line);
        line.setAttribute("r", 3);
        line.setAttribute("x1", taskLegendX);
        line.setAttribute("y1", taskLegendY);
        line.setAttribute("x2", taskLegendX + 2 * gap);
        line.setAttribute("y2", taskLegendY);
        line.setAttribute("stroke", svgColors[colorSelection]);
        line.setAttribute("stroke-width", 1);
        line.setAttribute("id", "legendline." + (*tli)->getFullId());
        line.setAttribute("onmouseover", "jmouseover(evt, '" + (*tli)->getFullId() + "')");
        line.setAttribute("onmouseout", "jmouseout(evt, '" + (*tli)->getFullId() + "')");
        // Add legend first circle mark
        circlemark = doc.createElement("circle");
        svg.appendChild(circlemark);
        circlemark.setAttribute("r", 3);
        circlemark.setAttribute("cx", taskLegendX);
        circlemark.setAttribute("cy", taskLegendY);
        circlemark.setAttribute("fill", "none");
        circlemark.setAttribute("stroke", svgColors[colorSelection]);
        circlemark.setAttribute("stroke-width", 1);
        // Add legend second circle mark
        taskLegendX += 2 * gap;
        circlemark = doc.createElement("circle");
        svg.appendChild(circlemark);
        circlemark.setAttribute("r", 3);
        circlemark.setAttribute("cx", taskLegendX);
        circlemark.setAttribute("cy", taskLegendY);
        circlemark.setAttribute("fill", "none");
        circlemark.setAttribute("stroke", svgColors[colorSelection]);
        circlemark.setAttribute("stroke-width", 1);
        // Add legend text
        taskLegendX += 2 * gap;
        taskName = doc.createElement("text");
        taskName.appendChild(doc.createTextNode(QString::number(i + 1) + " " + (*tli)->getName()));
        svg.appendChild(taskName);
        taskName.setAttribute("x", taskLegendX);
        taskName.setAttribute("y", taskLegendY + fontHeight / 2);
        taskName.setAttribute("fill", "black");
        taskName.setAttribute("font-family", "Courrier");
        taskName.setAttribute("font-size", fontHeight);
        taskName.setAttribute("onmouseover", "jmouseover(evt, '" + (*tli)->getFullId() + "')");
        taskName.setAttribute("onmouseout", "jmouseout(evt, '" + (*tli)->getFullId() + "')");
        taskName.setAttribute("id", "legendtext." + (*tli)->getFullId());

        for (it = scenarios.begin(); it != scenarios.end(); ++it)
        {
            // Add point only if above diagonal
            if ((*tli)->getStart(*it) >= getProject()->getScenario(*it)->getDate())
            {
                // Add comment, scenario name
                svg.appendChild(doc.createComment("Scenario : " + getProject()->getScenario(*it)->getName()));

                x = DATE_TO_X(getProject()->getScenario(*it)->getDate());
                y = DATE_TO_Y((*tli)->getStart(*it));

                // Add this point to polylinepoints QString.
                polylinepoints.append(" " + QString::number(x) + "," + QString::number(y) + " ");

                // Add one circle mark for each task and each milestones.
                circlemark = doc.createElement("circle");
                svg.appendChild(circlemark);
                circlemark.setAttribute("r", 3);
                circlemark.setAttribute("cx", x);
                circlemark.setAttribute("cy", y);
                circlemark.setAttribute("fill", "none");
                circlemark.setAttribute("stroke", svgColors[colorSelection]);
                circlemark.setAttribute("stroke-width", 1);
            }
        }

        // Create polyline using previously calculate polylinepoints QString.
        polyline = doc.createElement("polyline");
        svg.appendChild(polyline);
        polyline.setAttribute("points", polylinepoints.utf8());
        polyline.setAttribute("fill", "none");
        polyline.setAttribute("stroke", svgColors[colorSelection]);
        polyline.setAttribute("stroke-width", 1);
        polyline.setAttribute("id", "polyline." + (*tli)->getFullId());
        polyline.setAttribute("onmouseover", "jmouseover(evt, '" + (*tli)->getFullId() + "')");
        polyline.setAttribute("onmouseout", "jmouseout(evt, '" + (*tli)->getFullId() + "')");
    }

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
        milestonedate = doc.createElement("text");
        milestonedate.appendChild(doc.createTextNode(time2user(axeDate, getTimeFormat())));
        svg.appendChild(milestonedate);
        x = DATE_TO_X(axeDate);
        milestonedate.setAttribute("x", x);
        y = DATE_TO_Y(datemin) + gap;
        milestonedate.setAttribute("y", y);
        milestonedate.setAttribute("text-anchor", "end");
        milestonedate.setAttribute("fill", "black");
        milestonedate.setAttribute("rotate", 45);
        QString rotate = QString("rotate(");
        rotate += QString::number(-45) + " " + QString::number(x) + " " + QString::number(y) + ")";
        milestonedate.setAttribute("transform", rotate);
        milestonedate.setAttribute("font-family", "Courrier");
        milestonedate.setAttribute("font-size", fontHeight);

        // Add Y axe legent
        milestonedate = doc.createElement("text");
        milestonedate.appendChild(doc.createTextNode(time2user(axeDate, getTimeFormat())));
        svg.appendChild(milestonedate);
        milestonedate.setAttribute("x", margex - gap);
        milestonedate.setAttribute("y", DATE_TO_Y(axeDate) + fontHeight / 2);
        milestonedate.setAttribute("text-anchor", "end");
        milestonedate.setAttribute("fill", "black");
        milestonedate.setAttribute("font-family", "Courrier");
        milestonedate.setAttribute("font-size", fontHeight);
    }

    // Write xml string to out file.
    s << doc.toString();

    // Close file
    return close();
}

void SVGTimeTimeReport::inheritValues()
{
    SVGReport::inheritValues();

    SVGTimeTimeReport* parent = dynamic_cast<SVGTimeTimeReport*>(getParentReport());

    if (parent)
    {
        // In fact, for the moment, there is no SVGTimeTimeReport specfic attributes
    }
}

#endif
