#ifndef _EDITOR_VIEW_H_
#define _EDITOR_VIEW_H_

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

class QString;
class QVBoxLayout;

/**
 * Widget for editing the source code of the project
 * (uses the KTextEditor interface).
 * @short Editor widget
 * @author Lukas Tinkl <lukas.tinkl@suse.cz>
 */
class EditorView: public QWidget
{
    Q_OBJECT
public:
    /**
     * Standard constructor
     */
    EditorView( QWidget * parent = 0, const char * name = 0 );

    /**
     * Standard destructor
     */
    ~EditorView();

    /**
     * @return the path to the document being displayed
     */
    QString documentPath() const;

    /**
     * Load a document (project definition) from a file given by @p path
     *
     * @return true on success
     */
    bool loadDocument( const QString & path );

    /**
     * Close the current document, clear the view
     */
    void closeDocument();

private:
    /**
     * Initialize the KTextEditor components (Document and View)
     */
    bool init();

    /// text view
    KTextEditor::View * m_view;
    /// text document
    KTextEditor::Document * m_doc;
    /// path to the project file
    QString m_path;
    /// main layout
    QVBoxLayout * m_layout;
};

#endif
