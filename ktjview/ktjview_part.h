/*
 * ktjview_part.h - TaskJuggler Viewer
 *
 * Copyright (c) 2001, 2002 by Klaas Freitag <freitag@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef KTJVIEWPART_H
#define KTJVIEWPART_H

#include <kparts/part.h>

class QWidget;
class QCanvasView;
class QPainter;
class KURL;
class KTJGantt;
class KAboutData;
class Project;
/**
 * This is a "Part".  It that does all the real work in a KPart
 * application.
 *
 * @short Main Part
 * @author Klaas Freitag <freitag@kde.org>
 * @version 0.1
 */
class KTjviewPart : public KParts::ReadOnlyPart
{
    Q_OBJECT
public:
    /**
     * Default constructor
     */
    KTjviewPart(QWidget *parentWidget, const char *widgetName,
                    QObject *parent, const char *name, const QStringList &args);

    /**
     * Destructor
     */
    virtual ~KTjviewPart();


    static KAboutData *createAboutData();

public slots:

    void slReload();

protected:
    /**
     * This must be implemented by each part
     */
    virtual bool openFile();


protected slots:
    void fileOpen();
   void slChangeStatusBar( const QString& );

private:
    void setupActions();

    KTJGantt *m_gantt;
    Project *m_project;
    QWidget *m_parentWidget;
};

#endif // KTJVIEWPART_H
