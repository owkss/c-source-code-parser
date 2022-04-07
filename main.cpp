#include "mainwindow.h"
#include "typestorage.h"

#include <QMessageBox>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if (!Storage::check_directories())
    {
        QMessageBox::critical(nullptr, QObject::tr("Ошибка"), QObject::tr("Путь к CastXML не найден (%1)").arg(Storage::castxml_path()));
        //return 1;
    }

    MainWindow w;
    w.show();

    return a.exec();
}
