#include "codeeditor.h"

#include <QPainter>
#include <QTextBlock>

/* not mine */

CodeEditor::CodeEditor(QWidget *parent)
    : QPlainTextEdit(parent)
    , eow("~!@#$%^&()_{}|:\"<?,.;'[]\\=")
    , c(nullptr)
{
    this->setTabStopWidth(8* fontMetrics().width(' '));

    QFont font("Consolas");

    /*
    font.setFamily("TimesNewRoman");
    font.setWordSpacing(1.5);
    font.setLetterSpacing(QFont::AbsoluteSpacing,0.8);
    font.setStretch(120);
    */

    this->setFont(font);
    lineNumberArea = new LineNumberArea(this);

    connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &CodeEditor::updateRequest, this, &CodeEditor::updateLineNumberArea);
    connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);
    connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::cursorChanged);

    updateLineNumberAreaWidth(0);
    //highlightCurrentLine();
}

int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = std::max(1, blockCount());
    while (max >= 10)
    {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;
    return space;
}

void CodeEditor::setCompleter(QCompleter *c)
{
    if (c)
        QObject::disconnect(c, 0, this, 0);

    this->c = c;

    if (!c)
        return;

    c->setWidget(this);
    c->setCompletionMode(QCompleter::PopupCompletion);
    c->setCaseSensitivity(Qt::CaseInsensitive);
    QObject::connect(c, static_cast<void(QCompleter::*)(const QString&)>(&QCompleter::activated), this, &CodeEditor::insertCompletion);
}

void CodeEditor::updateLineNumberAreaWidth(int newBlockCount)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::cursorChanged()
{
    QList<QTextEdit::ExtraSelection> extras;
    QTextEdit::ExtraSelection extra;
    QTextCursor cur = this->textCursor();

    extra.cursor = this->textCursor();
    extra.format.setForeground(Qt::green);
    extra.format.setBackground(Qt::lightGray);
    extra.format.setFontWeight(100);

    QChar charN = this->document()->characterAt(cur.position());
    QChar charP = this->document()->characterAt(cur.position()-1);

    int p1 = cur.position()-1;
    int p2 = this->findComplementBracket(charP, -1);

    int n1 = cur.position();
    int n2 = this->findComplementBracket(charN, 1);

    if (p2 >= 0)
    {
        extra.cursor.setPosition(p2);
        extra.cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);

        extras<< extra;
        extra.cursor.setPosition(p1);
        extra.cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
        extras<< extra;
    }

    if (n2 >= 0)
    {
        extra.cursor.setPosition(n2);
        extra.cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);

        extras<< extra;
        extra.cursor.setPosition(n1);
        extra.cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
        extras<< extra;
    }

    this->setExtraSelections(extras);
}

void CodeEditor::insertCompletion(const QString &completion)
{
    if (c->widget() != this)
        return;

    QTextCursor tc = textCursor();
    int extra = completion.length() - c->completionPrefix().length();
    tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::EndOfWord);
    tc.insertText(completion.right(extra));
    setTextCursor(tc);
}

int CodeEditor::findComplementBracket(QChar ch, int toward)
{
    QChar toFind;

    if (toward == 1)
    {
        if (ch == '{')
        {
            toFind = '}';
        }
        else if (ch == '(')
        {
            toFind = ')';
        }
        else if (ch == '[')
        {
            toFind = ']';
        }
        else
        {
            return -1;
        }

        int count = 0;
        QString string = this->document()->toPlainText();
        int pos = this->textCursor().position();
        pos++;

        while (pos < string.length())
        {
            if (string[pos] == ch)
                count++;
            else if(string[pos] == toFind)
            {
                if (count)
                    count--;
                else
                    return pos;
            }

            pos++;
        }

        return -2;
    }
    else
    {
        if (ch == '}')
        {
            toFind = '{';
        }
        else if (ch == ')')
        {
            toFind = '(';
        }
        else if (ch == ']')
        {
            toFind = '[';
        }
        else
        {
            return -1;
        }

        int count = 0;
        QString string = this->document()->toPlainText();
        int pos = this->textCursor().position();
        pos -= 2;

        while (pos >= 0)
        {
            if (string[pos] == ch)
                count++;
            else if (string[pos] == toFind)
            {
                if (count)
                    count--;
                else
                    return pos;
            }

            pos--;
        }

        return -2;
    }
}

QString CodeEditor::textUnderCursor() const
{
    QTextCursor tc = textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    return tc.selectedText();
}

void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::keyPressEvent(QKeyEvent *e)
{
    QPlainTextEdit::keyPressEvent(e);
    if (c && c->popup()->isVisible())
    {
        // The following keys are forwarded by the completer to the widget
        switch (e->key())
        {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Escape:
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            e->ignore();
            return; // let the completer do default behavior
        default:
            break;
        }
    }


    QString string = this->document()->toPlainText();
    int pos = this->textCursor().position();

    if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return){

        int t = pos - 2;
        bool isExtraLine = (pos - 2 > 0 && string[pos - 2] == '{' && string[pos] == '}' ? true : false);
        while (t >= 0 && string[t] != '\n') t--;
        int countTab = 0;
        int countSpace = 0;
        t++;

        while (string[t] == '\t' || string[t] == ' ')
        {
            (string[t] == '\t' ? countTab++ : countSpace++);
            t++;
        }

        this->textCursor().insertText(QString(countTab, '\t'));
        this->textCursor().insertText(QString(countSpace, ' '));

        if (isExtraLine)
        {
            this->textCursor().insertText(QString("\t\n"));
            int posForCursor = this->textCursor().position() - 1;
            this->textCursor().insertText(QString(countTab, '\t'));
            this->textCursor().insertText(QString(countSpace, ' '));
            QTextCursor tempCur(this->document());
            tempCur.setPosition(posForCursor);
            this->setTextCursor(tempCur);
        }

        return;
    }

    QChar pairList[][2] = {
        {'(',')'},
        {'\'','\''},
        {'"','"'},
        {'{','}'},
        {'[',']'}
    };

    QChar key = e->key();
    for (int i = 0; i < 5; ++i)
    {

        if (key == pairList[i][1] && (this->document()->characterAt(this->textCursor().position()) == pairList[i][1]))
        {
            this->textCursor().deleteChar();
            return;
        }

        if (key == pairList[i][0])
        {
            this->textCursor().insertText(QString(pairList[i][1]));
            QTextCursor tempCur = this->textCursor();
            tempCur.setPosition(tempCur.position() - 1);
            this->setTextCursor(tempCur);
            return;
        }
    }

    bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_E); // CTRL+E
    //if (!c || !isShortcut) QPlainTextEdit::keyPressEvent(e);

    const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    if (!c || (ctrlOrShift && e->text().isEmpty()))
        return;

    bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
    QString completionPrefix = textUnderCursor();

    if (!isShortcut && (hasModifier || e->text().isEmpty()|| completionPrefix.length() < 2 || eow.contains(e->text().right(1))))
    {
        c->popup()->hide();
        return;
    }

    if (completionPrefix != c->completionPrefix())
    {
        c->setCompletionPrefix(completionPrefix);
        c->popup()->setCurrentIndex(c->completionModel()->index(0, 0));
    }

    QRect cr = cursorRect();
    cr.setWidth(c->popup()->sizeHintForColumn(0) + c->popup()->verticalScrollBar()->sizeHint().width());
    c->complete(cr);
}

void CodeEditor::focusInEvent(QFocusEvent *e)
{
    if (c)
        c->setWidget(this);
    QPlainTextEdit::focusInEvent(e);
}

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly())
    {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Qt::yellow).lighter(160);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int)blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom())
    {
        if (block.isVisible() && bottom >= event->rect().top())
        {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(), Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}
