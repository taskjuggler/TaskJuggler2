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

#include <vector>
#include <map>

#include <qstringlist.h>
#include <qcolor.h>
#include <qfont.h>

#include "ReportLayers.h"

class QObject;
class QPainter;
class QRect;
class QCanvas;
class Project;
class Task;
class Resource;
class QtReport;
class QtReportElement;
class TjObjPosTable;
class TjGanttZoomStep;
class Interval;

class TjGanttChart {
public:
    enum ScaleMode { manual = 0, fitSize, autoZoom };

    TjGanttChart(QObject* obj);
    ~TjGanttChart();

    void setProjectAndReportData(const QtReportElement* r);
    void setSizes(const TjObjPosTable* opt, int headerHeight, int chartHeight,
                  int width, int minRowHeight);
    void setDPI(int dx, int dy);

    void setColor(const char* name, QColor col);

    int calcHeaderHeight();
    void setHeaderHeight(int hh);
    int calcLegendHeight(int width);
    void generate(ScaleMode scaleMode);
    void paintHeader(const QRect& clip, QPainter* p, bool dbuf = false);
    void paintChart(int x, int y, const QRect& clip, QPainter* p,
                    bool dbuf = false);
    void paintLegend(const QRect& clip, QPainter* p, bool dbuf = false);

    void generateLegend(int width, int height);

    QCanvas* getHeaderCanvas() const { return header; }
    QCanvas* getChartCanvas() const { return chart; }
    QCanvas* getLegendCanvas() const { return legend; }

    bool zoomIn();
    bool zoomOut();

    Interval stepInterval(time_t ref) const;
    QString stepIntervalName(time_t ref) const;

    int time2x(time_t t) const;
    time_t x2time(int x) const;

    int getWidth() const { return width; }

private:
    TjGanttChart() { }

    void calcStepSizes();
    void generateHeaderAndGrid();
    void generateHeaderLine(int y);
    void drawHeaderCell(int x, int y, int xe, const QString label, bool first);
    void generateGanttBackground();
    void markNonWorkingHoursOnBackground();
    void markNonWorkingDaysOnBackground();
    void markBoundary(int x, bool now = false, int layer = TJRL_GRIDLINES);
    void markHourBoundaries(int distance);
    void markDayBoundaries();
    void markWeekBoundaries();
    void markMonthsBoundaries();
    void markQuarterBoundaries();

    void generateGanttElements();
    void drawTask(const Task* t, const Resource* r);
    void drawTaskShape(int start, int end, int centerY, int height,
                       int barWidth, bool outlineOnly, QCanvas* canvas);
    void drawMilestoneShape(int centerX, int centerY, int height,
                            bool outlineOnly, QCanvas* canvas);
    void drawContainterShape(int start, int end, int centerY, int height,
                             bool outlineOnly, QCanvas* canvas);
    void drawDependencies(const Task* t1);
    void drawTaskResource(const Resource* r, const Task* t);
    void drawResource(const Resource* r);
    void drawResourceTask(const Task* t, const Resource* r);
    void drawResourceLoadColum(const Resource* r, const Task* t, time_t start,
                               time_t end, int rY);

    void zoomToFitWindow(int width, time_t duration);
    void allTasksInterval();

    int generateLegend();

    int mmToXPixels(double mm);
    int mmToYPixels(double mm);
    int pointsToYPixels(double pts);

    QCanvas* header;
    QCanvas* chart;
    QCanvas* legend;

    // Vertical and horizontal resolution
    int dpiX;
    int dpiY;

    int lineWidth;

    time_t startTime;
    time_t endTime;

    time_t unclippedEndTime;
    bool clipped;

    const TjObjPosTable* objPosTable;

    int headerMargin;
    int headerHeight;
    int chartHeight;
    int width;
    int minRowHeight;

    struct ltstr
    {
        bool operator()(const char* s1, const char* s2) const
        {
            return strcmp(s1, s2) < 0;
        }
    };
    std::map<const char*, QColor, ltstr> colors;

    std::vector<TjGanttZoomStep*> zoomSteps;
    unsigned int currentZoomStep;

    const Project* project;
    const QtReport* reportDef;
    const QtReportElement* reportElement;
    int scenario;

    QFont headerFont;
    QFont legendFont;
    QStringList legendLabels;
    int maxLegendLabelWidth;
    int legendLabelHeight;
    int legendLabelRows;
} ;

#endif

