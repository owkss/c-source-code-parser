#include "mainwindow.h"
#include "exportview.h"
#include "typestorage.h"

#include <QTimer>
#include <QGridLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setMinimumSize(640, 480);
    create_ui();

    Storage *s = Storage::instance();
    connect(this, &MainWindow::try_parse, s, &Storage::try_parse);
    connect(s, &Storage::parse_error, this, &MainWindow::on_parse_error);
    connect(s, &Storage::count_changed, m_view, &ExportView::refresh);
}

MainWindow::~MainWindow()
{}

void MainWindow::on_parse_error(const QString &errstr)
{
    QMessageBox::warning(this, tr("Ошибка разбора"), errstr);
}

void MainWindow::create_ui()
{
    QWidget *container = new QWidget(this);
    QGridLayout *grid = new QGridLayout;
    m_view = new ExportView(container);
    m_choose_file_btn = new QPushButton(tr("Выбрать файл"), container);

    if (container->layout())
        delete container->layout();

    grid->addWidget(m_view, 0, 0, 1, 2);
    grid->addWidget(m_choose_file_btn, 1, 1);
    container->setLayout(grid);

    setCentralWidget(container);

    connect(m_choose_file_btn, &QPushButton::clicked, this, &MainWindow::choose_file);
}

void MainWindow::choose_file()
{
    QFileDialog fd(this);
    fd.setNameFilters({ "*.c" });
    fd.setFileMode(QFileDialog::ExistingFile);
    fd.setAcceptMode(QFileDialog::AcceptOpen);
    fd.setDirectory(QApplication::applicationDirPath());
    fd.setWindowTitle(tr("Выбор исходного файла"));

    if (fd.exec() != QFileDialog::Accepted)
        return;

    QString fn = fd.selectedFiles().at(0);
    emit try_parse(fn);
}

