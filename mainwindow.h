#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class ExportView;
class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void on_parse_error(const QString &errstr);

private:
    void create_ui();

    ExportView *m_view = nullptr;
    QPushButton *m_choose_file_btn = nullptr;

    void choose_file();

signals:
    void try_parse(const QString &filename);
};
#endif // MAINWINDOW_H
