#ifndef TYPESTORAGE_H
#define TYPESTORAGE_H

#include "type.h"

#include <map>
#include <iostream>

#include <QDir>
#include <QObject>

class Storage : public QObject
{
    Q_OBJECT
public:
    explicit Storage(QObject *parent = nullptr);
    ~Storage();

    static Storage *instance();
    static bool check_directories();
    static QString castxml_path();

    QString path() const { return m_local_data_dir.path(); }
    QString make_path(const QString &filename) const { return m_local_data_dir.path() + "/" + filename; };

    bool create_type(const QString &declaration);
    void try_parse(const QString &filename);

    bool remove_variable(AbstractType *v);
    bool remove_variable(const std::string &name);
    bool contains_variable(const std::string &other) const noexcept;
    void clear();

    std::vector<std::pair<std::string, std::shared_ptr<AbstractType>>> &variable() { return m_variables; }

private:
    static Storage *self;

    void create_directory();
    void load_data();
    void save_data();

    /* Разбор XML */
    bool parse(char *xml, const std::string &source_filename);

    std::vector<std::pair<std::string, std::shared_ptr<AbstractType>>> m_variables;

    QDir m_local_data_dir;

signals:
    void count_changed();
    void parse_error(const QString &errstr); // Отправка только при ошибках разбора XML
};

#endif // TYPESTORAGE_H
