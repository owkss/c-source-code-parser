#ifndef EXPORTMODEL_H
#define EXPORTMODEL_H

#include <QAbstractItemModel>

#include "typestorage.h"

class ExportModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum COLUMNS { NAME, TYPE, SIZE, OFFSET, N_COLUMNS };

    explicit ExportModel(QObject *parent = nullptr);

    void refresh();

    // Read-only interface
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    bool hasChildren(const QModelIndex &parent) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    std::vector<std::pair<std::string, std::shared_ptr<AbstractType>>> &m_vars;
};

#endif // EXPORTMODEL_H
