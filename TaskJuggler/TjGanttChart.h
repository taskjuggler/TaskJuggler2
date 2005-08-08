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
#ifndef _TjGanttChart_h_
#define _TjGanttChart_h_

#include <time.h>

#include <qcolor.h>

#include "ReportLayers.h"

class QObject;
class QPainter;
class QRect;
class QCanvas;
class Project;
class TaskList;
class ResourceList;
class QtReport;
class QtReportElement;

class TjGanttChart {
public:
    enum ScaleMode { fitSize = 0, fitInterval, manual };

    TjGanttChart(QObject* obj);
    ~TjGanttChart();

    void setProjectAndReportData(const QtReportElement* r,
                                 const TaskList* tl, const ResourceList* rl);
    void setSizes(int headerHeight, int chartHeight, int width);

    void setColors(const QColor& hBackground, const QColor& cBackground,
                   const QColor& altBackground,
                   const QColor& base,
                   const QColor& base2, const QColor& mid);
    void setScaleMode(ScaleMode sm) { scaleMode = sm; }

    void generate();
    void paintHeader(const QRect& clip, QPainter* p, bool dbuf = false);
    void paintChart(const QRect& clip, QPainter* p, bool dbuf = false);

private:
    enum StepUnits { hour = 0, day, week, month, quarter, year };

    TjGanttChart() { }

    void generateHeaderAndGrid();
    void generateHourHeader(int y);
    void generateDayHeader(int y);
    void generateWeekHeader(int y);
    void generateMonthHeader(int y, bool withYear);
    void generateQuarterHeader(int y);
    void generateYearHeader(int y);
    void generateGanttBackground();
    void markNonWorkingHoursOnBackground();
    void markNonWorkingDaysOnBackground();
    void markBoundary(int x, bool now = false, int layer = TJRL_GRIDLINES);
    void markHourBoundaries(int distance);
    void markDayBoundaries();
    void markWeekBoundaries();
    void markMonthsBoundaries();
    void markQuarterBoundaries();

    int time2x(time_t t) const;
    time_t x2time(int x) const;

    void setBestStepUnit();

    QCanvas* header;
    QCanvas* chart;

    time_t startTime;
    time_t endTime;

    int headerHeight;
    int chartHeight;
    int width;

    QColor headerBackgroundCol;
    QColor chartBackgroundCol;
    QColor altBackgroundCol;
    QColor baseCol;
    QColor base2Col;
    QColor midCol;

    ScaleMode scaleMode;

    int pixelPerYear;
    StepUnits stepUnit;
    static const int minStepHour;
    static const int minStepDay;
    static const int minStepWeek;
    static const int minStepMonth;
    static const int minStepQuarter;
    static const int minStepYear;
    static const int zoomSteps[];
    unsigned int currentZoomStep;

    const Project* project;
    const QtReport* reportDef;
    const QtReportElement* reportElement;
    const TaskList* taskList;
    const ResourceList* resourceList;
    int scenario;
} ;

#endif

