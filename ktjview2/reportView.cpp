// local includes
#include "reportView.h"

// TJ includes
#include "TaskList.h"

ReportView::ReportView( QWidget *parent, const char *name )
    : QicsTable( 0, parent, name ), m_model( 0 )
{
    setReadOnly( true );

    m_model = new QicsDataModelDefault();
}

ReportView::~ReportView()
{
    delete m_model;
}

void ReportView::createTaskReport( Project * proj )
{
    m_model->clearModel();

    TaskListIterator tasks = proj->getTaskListIterator();



    setDataModel( m_model );
}

void ReportView::createResourceReport( Project * proj )
{
    m_model->clearModel();

    setDataModel( m_model );
}



#include "reportView.moc"
