#include "exportmodel.h"
#include "typestorage.h"

ExportModel::ExportModel(QObject *parent)
    : QAbstractItemModel(parent)
    , m_vars(Storage::instance()->variable())
{

}

void ExportModel::refresh()
{
    beginResetModel();
    endResetModel();
}

QModelIndex ExportModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0) // Родительский индекс может быть только в первой колонке
        return QModelIndex();

    AbstractType *t = static_cast<AbstractType*>(parent.internalPointer());
    if (ComplexType *cv = dynamic_cast<ComplexType*>(t))
    {
        if (std::size_t(row) < cv->members.size())
            return createIndex(row, column, cv->members[row]);
    }
    else
    {
        if (std::size_t(row) < m_vars.size())
            return createIndex(row, column, m_vars.at(row).second.get());
    }

    return QModelIndex();
}

QModelIndex ExportModel::parent(const QModelIndex &index) const
{
    AbstractType *child_type = static_cast<AbstractType*>(index.internalPointer());
    AbstractType *parent_data = child_type ? child_type->context : nullptr;

    if (!parent_data)
        return QModelIndex();

    int row = 0; // Поиск родительского типа в списке его контекста
    if (ComplexType *cv = dynamic_cast<ComplexType*>(parent_data->context))
    {
        int i = 0;
        for (AbstractType *m : cv->members)
        {
            if (m == parent_data)
                break;
            ++i;
        }

        row = i;
    }

    return createIndex(row, 0, parent_data);
}

int ExportModel::rowCount(const QModelIndex &parent) const
{
    AbstractType *t = static_cast<AbstractType*>(parent.internalPointer());
    if (!t)
        return m_vars.size();
    if (ComplexType *cv = dynamic_cast<ComplexType*>(t))
        return cv->members.size();
    return 0;
}

int ExportModel::columnCount(const QModelIndex &) const
{
    return N_COLUMNS;
}

QVariant ExportModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    AbstractType *t = static_cast<AbstractType*>(index.internalPointer());
    switch (role)
    {
    case Qt::DisplayRole:
    {
        switch (index.column())
        {
            case NAME: return QString::fromStdString(t->full_name());
            case TYPE: return QString::fromStdString(t->type);
            case SIZE: return (t->qualifiers.is_array ? quint64(t->size / 8) * t->count : quint64(t->size / 8));
            case OFFSET: return quint64(t->offset);
            default: return QVariant();
        }
    }
    case Qt::ToolTipRole:
    {
        if (TrivialType *tv = dynamic_cast<TrivialType*>(t))
        {
            if (tv->bits < tv->size)
                return tr("Битовое поле: %1 %2").arg(tv->bits).arg(tv->bits >= 2 && tv->bits <= 4 ? tr("бита") : tr("бит"));
        }

        break;
    }
    default:
        return QVariant();
    }

    return QVariant();
}

QVariant ExportModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
    {
        switch (section)
        {
        case NAME:
            return tr("Имя");
        case TYPE:
            return tr("Тип");
        case SIZE:
            return tr("Размер (байт)");
        case OFFSET:
            return tr("Смещение (бит)");
        default:
            return QVariant();
        }
    }
    else
    {
        return section + 1;
    }
}

bool ExportModel::hasChildren(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return !m_vars.empty();
    AbstractType *t = static_cast<AbstractType*>(parent.internalPointer());
    if (ComplexType *cv = dynamic_cast<ComplexType*>(t))
        return !cv->members.empty();
    return false;
}

Qt::ItemFlags ExportModel::flags(const QModelIndex &) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
