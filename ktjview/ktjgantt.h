#ifndef KTJGANTT_H
#define KTJGANTT_H

#include <qsplitter.h>
class QWidget;
class QCanvasView;
class QPainter;
class KURL;
class KTVTaskTable;
class KTVTaskCanvasView;
class KAboutData;
class Project;


/**
 *
 * @short TaskJuggler Gantt Viewer
 * @author Klaas Freitag <freitag@kde.org>
 * @version 0.1
 */
class KTJGantt : public QSplitter
{
    Q_OBJECT
public:
    /**
     * Default constructor
     */
    KTJGantt( QWidget *parentWidget, const char *widgetName );

    /**
     * Destructor
     */
    virtual ~KTJGantt();


   void showProject( Project * );
   
protected:

private:
   KTVTaskCanvasView *m_canvas;
   KTVTaskTable *m_table;
};

#endif // KTJVIEWPART_H
