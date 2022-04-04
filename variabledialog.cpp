#include "variabledialog.h"
#include "typestorage.h"
#include "highlighter.h"
#include "codeeditor.h"

#include <QDebug>
#include <QTextEdit>
#include <QLineEdit>
#include <QComboBox>
#include <QGroupBox>
#include <QMessageBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QApplication>

VariableDialog::VariableDialog(QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f)
{
    create_ui();
}

VariableDialog::~VariableDialog()
{
    disconnect();
}

void VariableDialog::create_ui()
{
    setMinimumSize(480, 360);
    setWindowTitle(tr("Добавление новой экспортируемой переменной"));

    m_source_group = new QGroupBox(tr("Исходный код"), this);
    m_source_edit = new CodeEditor(this);
    m_ok_btn = new QPushButton(this);
    m_cancel_btn = new QPushButton(this);
    m_completer = new QCompleter(this);
    m_highlighter = new Highlighter(m_source_edit->document());

    m_completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setWrapAround(false);
    m_source_edit->setCompleter(m_completer);

    if (layout())
        delete layout();

    QVBoxLayout *main_layout = new QVBoxLayout;
    QHBoxLayout *buttons_layout = new QHBoxLayout;

    QVBoxLayout *vl = new QVBoxLayout;
    vl->addWidget(m_source_edit);
    m_source_group->setLayout(vl);

    buttons_layout->addWidget(m_ok_btn);
    buttons_layout->addWidget(m_cancel_btn);

    main_layout->addWidget(m_source_group);
    main_layout->addLayout(buttons_layout);
    setLayout(main_layout);

    m_ok_btn->setText(tr("Добавить"));
    m_cancel_btn->setText(tr("Отмена"));
    m_source_edit->setPlaceholderText(tr("Исходный код типа экспортируемой переменной. Также, должны быть объявлены все вложенные структуры. Переменных может быть несколько. Рекурсивные структуры не поддерживаются."));

    m_source_edit->setWhatsThis(tr("Здесь записываются все определения типов и объявления переменных. Например, \"int g_int;\" или \n\"struct TYPE\n{\n\tdouble field;\n};\n\nstruct TYPE global_type;\"\""));
    m_ok_btn->setWhatsThis(tr("Парсинг и добавление переменной"));
    m_cancel_btn->setWhatsThis(tr("Закрытие диалогового окна"));

    connect(m_ok_btn, &QPushButton::clicked, this, &VariableDialog::ok_button_clicked);
    connect(m_cancel_btn, &QPushButton::clicked, this, &VariableDialog::cancel_button_clicked);
    connect(Storage::instance(), &Storage::parse_error, this, &VariableDialog::fill_journal);
}

void VariableDialog::ok_button_clicked()
{
    QString source_text = "#include <stdint.h>\n";
    source_text += m_source_edit->toPlainText();

    QApplication::setOverrideCursor(Qt::WaitCursor);

    Storage *s = Storage::instance();
    const bool result = s->create_type(source_text);
    emit need_update();

    QApplication::restoreOverrideCursor();

    if (!m_journal.isEmpty())
    {
        QDialog d(this);
        d.setWindowTitle(tr("Ошибка разбора исходного кода"));
        QTextEdit *edit = new QTextEdit(&d);
        edit->setPlainText(m_journal);
        QVBoxLayout *vl = new QVBoxLayout(&d);
        vl->addWidget(edit);
        d.setMinimumSize(640, 480);

        d.exec();
        m_journal.clear();
    }
    else
    {
        if (result)
        {
            done(QDialog::Accepted);
        }
        else
        {
            QMessageBox::critical(this, tr("Ошибка"), tr("Произошла неизвестная внутренняя ошибка. Проверьте входные данные ещё раз"));
        }
    }
}

void VariableDialog::cancel_button_clicked()
{
    done(QDialog::Rejected);
}

void VariableDialog::fill_journal(const QString &errstr)
{
    m_journal += errstr;
    m_journal += "\n";
}
