#ifndef KTJVIEWPART_H
#define KTJVIEWPART_H

#include <kparts/part.h>

class QWidget;
class QCanvasView;
class QPainter;
class KURL;
class KTJGantt;
class KAboutData;

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

protected:
    /**
     * This must be implemented by each part
     */
    virtual bool openFile();


protected slots:
    void fileOpen();


private:
   KTJGantt *m_widget;
};

#endif // KTJVIEWPART_H
