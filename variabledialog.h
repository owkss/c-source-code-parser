#ifndef VARIABLEDIALOG_H
#define VARIABLEDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QComboBox;
class QGroupBox;
class CodeEditor;
class QCompleter;
class QPushButton;
class Highlighter;
QT_END_NAMESPACE

class VariableDialog : public QDialog
{
    Q_OBJECT
public:
    VariableDialog(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    ~VariableDialog();

private:
    void create_ui();
    QGroupBox *m_source_group = nullptr;
    CodeEditor *m_source_edit = nullptr;
    QCompleter *m_completer = nullptr;
    Highlighter *m_highlighter = nullptr;
    QPushButton *m_ok_btn = nullptr;
    QPushButton *m_cancel_btn = nullptr;

    QString m_journal;

    void ok_button_clicked();
    void cancel_button_clicked();

    void fill_journal(const QString &errstr);

signals:
    void need_update();
};

#endif // VARIABLEDIALOG_H
