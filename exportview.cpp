#include "exportview.h"
#include "exportmodel.h"

ExportView::ExportView(QWidget *parent)
    : QTreeView(parent)
{
    setAnimated(true);
    setContextMenuPolicy(Qt::CustomContextMenu);

    m_model = new ExportModel(this);
    setModel(m_model);
}

void ExportView::refresh()
{
    m_model->refresh();
}
