#ifndef _EDITOR_VIEW_H_
#define _EDITOR_VIEW_H_

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

class KURL;
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

    virtual ~EditorView();

    /**
     * @return the URL of the document being displayed
     */
    KURL url() const;

    /**
     * Load a document (project definition) from @p url
     *
     * @return true on success
     */
    bool loadDocument( const KURL & url );

    /**
     * Close the current document, clear the view
     */
    void closeDocument();

    /**
     * @return KTextEditor::Document bound to #view()
     */
    KTextEditor::Document * doc() const
        { return m_view->document(); }

    /**
     * @return KTextEditor::View bound to this component
     */
     KTextEditor::View * view() const
        { return m_view; }

signals:
    /**
     * Emitted when the text (source of the project) changes
     */
    void textChanged();

    /**
     * Emitted when the cursor changes its position
     */
    void cursorChanged();

    /**
     * Emitted when the selection changes
     */
    void selectionChanged();

private:
    /**
     * Initialize the KTextEditor components (Document and View)
     */
    bool init();

    /// text view
    KTextEditor::View * m_view;
    /// main layout
    QVBoxLayout * m_layout;
};

#endif
