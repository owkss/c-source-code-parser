#ifndef EXPORTVIEW_H
#define EXPORTVIEW_H

#include <QTreeView>

QT_BEGIN_NAMESPACE
class ExportModel;
QT_END_NAMESPACE

class ExportView : public QTreeView
{
    Q_OBJECT
public:
    ExportView(QWidget *parent = nullptr);

    void refresh();

private:
    ExportModel *m_model = nullptr;
};

#endif // EXPORTVIEW_H
