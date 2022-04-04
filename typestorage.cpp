#include "typestorage.h"

#include <cassert>

#include <QFile>
#include <QDebug>
#include <QProcess>
#include <QTextStream>
#include <QDirIterator>
#include <QStandardPaths>

Storage *Storage::self = nullptr;
Storage::Storage(QObject *parent)
    : QObject(parent)
{
    assert(self == nullptr);

    create_directory();
    load_data();

    self = this;
}

Storage::~Storage()
{
    save_data();
    self = nullptr;
}

void Storage::create_directory()
{
    QStringList path_list = QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation);
    if (path_list.empty())
        m_local_data_dir.setPath(".");
    else
        m_local_data_dir.setPath(path_list.at(0));

    if (m_local_data_dir.exists() == false)
        m_local_data_dir.mkpath(m_local_data_dir.path());

    QDirIterator iter(path(), {"*.c", "*.xml"}, QDir::Files, QDirIterator::Subdirectories);
    while (iter.hasNext())
    {
        QFile::remove(iter.next());
    }
}

void Storage::load_data()
{

}

void Storage::save_data()
{

}

Storage *Storage::instance()
{
    static Storage storage;
    return self;
}

bool Storage::check_directories()
{
    QString os_path;
    QString os_exe;
#ifdef _WIN32
    os_path = "./windows";
    os_exe = "castxml.exe";
#else
    os_path = "./linux";
    os_exe = "castxml";
#endif

    QString path = QString("%1/castxml/bin/%2").arg(os_path).arg(os_exe);
    return QFile::exists(path);
}

bool Storage::create_type(const QString &declaration)
{
    bool result = false;
    QFileInfo src_fi(make_path("scr_tmp.c"));
    QFileInfo xml_fi(make_path("xml_tmp.xml"));

    QFile src_file(src_fi.absoluteFilePath());
    if (src_file.open(QIODevice::WriteOnly))
    {
        QTextStream stream(&src_file);
        stream << declaration;

        src_file.close();

        QDir dir;
        QProcess p;
#ifdef _WIN32
        dir.setPath("./windows/castxml/bin");
        p.setWorkingDirectory(dir.absolutePath());
        p.setProgram(dir.absolutePath() + "/castxml.exe");
#else
        dir.setPath("./linux/castxml/bin");
        p.setWorkingDirectory(dir.absolutePath());
        p.setProgram(dir.absolutePath() + "/castxml");
#endif
        p.setArguments({"--castxml-output=1", src_fi.absoluteFilePath(), "-o", xml_fi.absoluteFilePath()});
        connect(&p, &QProcess::readyReadStandardError, this, [this, &p]() { emit parse_error(p.readAllStandardError()); });
        p.start();

        if (p.waitForFinished())
        {
            QFile xml_file(xml_fi.absoluteFilePath());
            if (xml_file.open(QIODevice::ReadOnly))
            {
                QByteArray xml = xml_file.readAll();
                xml_file.remove();

                std::string source_filename = src_fi.fileName().toStdString();
                result = parse(xml.data(), source_filename.data());
            }
        }

        src_file.remove();

        QByteArray stdout_str = p.readAllStandardOutput().simplified();
        QByteArray stderr_str = p.readAllStandardError().simplified();

        if (!stdout_str.isEmpty())
            emit parse_error(stdout_str);
        if (!stderr_str.isEmpty())
            emit parse_error(stderr_str);
    }

    if (result)
        emit count_changed();

    return result;
}

void Storage::try_parse(const QString &filename)
{
    clear();
    QFileInfo xml_fi(make_path("xml_tmp.xml"));

    QDir dir;
    QProcess p;
#ifdef _WIN32
    dir.setPath("./windows/castxml/bin");
    p.setWorkingDirectory(dir.absolutePath());
    p.setProgram(dir.absolutePath() + "/castxml.exe");
#else
    dir.setPath("./linux/castxml/bin");

    if (!QFile::exists(dir.absolutePath()))
    {
        dir.setPath("/usr/bin");
    }

    p.setWorkingDirectory(dir.absolutePath());
    p.setProgram(dir.absolutePath() + "/castxml");
#endif
    p.setArguments({"--castxml-output=1", filename, "-o", xml_fi.absoluteFilePath()});
    connect(&p, &QProcess::readyReadStandardError, this, [this, &p]() { emit parse_error(p.readAllStandardError()); });
    p.start();

    bool result = false;
    if (p.waitForFinished())
    {
        QFile xml_file(xml_fi.absoluteFilePath());
        if (xml_file.open(QIODevice::ReadOnly))
        {
            QByteArray xml = xml_file.readAll();
            xml_file.remove();

            std::string source_filename = QFileInfo(filename).fileName().toStdString();
            result = parse(xml.data(), source_filename.data());
        }
    }

    QByteArray stdout_str = p.readAllStandardOutput().simplified();
    QByteArray stderr_str = p.readAllStandardError().simplified();

    if (!stdout_str.isEmpty())
        emit parse_error(stdout_str);
    if (!stderr_str.isEmpty())
        emit parse_error(stderr_str);

    if (result)
        emit count_changed();
}

bool Storage::remove_variable(AbstractType *v)
{
    for (auto it = m_variables.begin(); it != m_variables.end(); ++it)
    {
        auto ptr = it->second;
        if (ptr.get() == v)
        {
            m_variables.erase(it);
            emit count_changed();
            return true;
        }
    }

    return false;
}

bool Storage::remove_variable(const std::string &name)
{
    for (auto it = m_variables.begin(); it != m_variables.end(); ++it)
    {
        if (it->first == name)
        {
            m_variables.erase(it);
            emit count_changed();
            return true;
        }
    }

    return false;
}

bool Storage::contains_variable(const std::string &other) const noexcept
{
    for (auto it = m_variables.cbegin(); it != m_variables.cend(); ++it)
    {
        // Как мы помним, в языке Си регистрозависимые названия переменных, поэтому применение обычного оператора сравнения вполне оправдано
        if (it->first == other)
        {
            return true;
        }
    }

    return false;
}

void Storage::clear()
{
    m_variables.clear();
}
