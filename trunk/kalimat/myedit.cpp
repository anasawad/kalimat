/**************************************************************************
**   The Kalimat programming language
**   Copyright 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "myedit.h"
#include "Lexer/lexer_incl.h"
#include "Lexer/kalimatlexer.h"
#include "mainwindow.h"
bool isAfterNumber(QTextEdit *edit)
{
    bool replace = false;
    if(edit->textCursor().position()>=1)
    {
        QChar at = edit->document()->characterAt(edit->textCursor().position()-1);
        if(at.isDigit())
        {
            replace = true;
        }
    }
    return replace;
}
bool isAfterArabicNumber(QTextEdit *edit)
{
    bool replace = false;
    if(edit->textCursor().position()>=1)
    {
        QChar at = edit->document()->characterAt(edit->textCursor().position()-1);
        if(at.unicode() >= L'٠' && at.unicode() < L'٩' )
        {
            replace = true;
        }
    }
    return replace;
}
MyEdit::MyEdit(MainWindow *owner) : QTextEdit()
{
    this->owner = owner;
    connect(this, SIGNAL(textChanged()), SLOT(textChangedEvent()));
    connect(this,  SIGNAL(cursorPositionChanged()), SLOT(selectionChangedEvent()));
    _line = _column = 0;
}

void MyEdit::keyPressEvent(QKeyEvent *ev)
{
    static QString arabComma = QString::fromWCharArray(L"،");
    if(ev->key() == Qt::Key_Tab)
    {
        tabBehavior();
    }
    else if(ev->key() == Qt::Key_Backtab)
    {
         shiftTabBehavior();
    }
    else if(ev->key() == Qt::Key_Return)
    {
        enterKeyBehavior();
    }
    else if(ev->key() == Qt::Key_Left)
    {
        QKeyEvent *otherEvent = new QKeyEvent(ev->type(), Qt::Key_Right, ev->modifiers(), ev->text(), ev->isAutoRepeat(), ev->count());
        QTextEdit::keyPressEvent(otherEvent);
    }
    else if(ev->key() == Qt::Key_Right)
    {
        QKeyEvent *otherEvent = new QKeyEvent(ev->type(), Qt::Key_Left, ev->modifiers(), ev->text(), ev->isAutoRepeat(), ev->count());;
        QTextEdit::keyPressEvent(otherEvent);
    }
    else if(ev->text() == "," || ev->text() == arabComma)
    {
        bool rightAfterNumber = isAfterNumber(this);
        QTextEdit::keyPressEvent(ev);
        if(rightAfterNumber)
        {
            QKeyEvent *otherEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_Space, 0, " ", false, 1);
            QTextEdit::keyPressEvent(otherEvent);
        }
    }
    else if(ev->text() == ".")
    {
        bool replace = isAfterArabicNumber(this);
        if(!replace)
            QTextEdit::keyPressEvent(ev);
        else
            this->insertPlainText(QString::fromWCharArray(L"٫"));
    }

    else
    {
        QTextEdit::keyPressEvent(ev);
    }
}

void MyEdit::tabBehavior()
{
    this->insertPlainText("    ");
}

void MyEdit::shiftTabBehavior()
{
    QTextCursor c = textCursor();
    int a = c.selectionStart();
    int b = c.selectionEnd();


    int aLine, aCol, bLine, bCol;

    lineTracker.lineColumnOfPos(a, aLine, aCol);
    lineTracker.lineColumnOfPos(b, bLine, bCol);

    int lineStart, lineEnd;
    lineTracker.linesFromTo(a, b, lineStart, lineEnd);

    int erasedFromA = 0, erasedFromB = 0;
    for(int i=lineStart; i<=lineEnd; i++)
    {
        LineInfo li = lineTracker.line(i);
        QString txt = textOfLine(li);
        int toErase = 0;
        for (int j=0; j<4; j++)
        {
            if(j==txt.length())
                break;
            if(txt[j]!=' ') // todo: handle real tab characters
                break;
            toErase++;
        }
        if(i == lineStart)
            erasedFromA = toErase;
        if(i == lineEnd)
            erasedFromB = toErase;
        if(toErase != 0)
        {
            QTextCursor c2 = textCursor();
            c2.setPosition(li.start);
            c2.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, toErase);
            setTextCursor(c2);
            c2.removeSelectedText();

            // todo: This recalculates line info every time, we could optimize.
            lineTracker.setText(this->document()->toPlainText());
        }
    }
    lineTracker.setText(this->document()->toPlainText());

    aCol -= erasedFromA;
    if(aCol<0)
        aCol = 0;
    bCol -= erasedFromB;
    if(bCol<0)
        bCol = 0;

    int aPos = lineTracker.posFromLineColumn(aLine, aCol);
    int bPos = lineTracker.posFromLineColumn(bLine, bCol);

    c.setPosition(aPos);
    c.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, bPos - aPos);
    setTextCursor(c);
}

bool tokensEqual(QVector<Token> toks, int ids[], int count)
{
    if(toks.count() != count)
        return false;
    for(int i=0; i<count; i++)
    {
        if(toks[i].Type != ids[i])
            return false;
    }
    return true;
}
bool tokensBeginEnd(QVector<Token> toks, int ids1[], int ids2[], int count1, int count2)
{
    if(toks.count()< count1 || toks.count()< count2)
        return false;
    for(int i=0; i<count1; i++)
    {
        if(toks[i].Type != ids1[i])
            return false;
    }

    for(int i=count2-1, j = toks.count() - 1; i>=0; i--,j--)
    {
        if(toks[j].Type != ids2[i])
            return false;
    }
    return true;
}

void MyEdit::enterKeyBehavior()
{
    LineInfo li = currentLine();
    QString txt = textOfLine(li);
    txt = txt.trimmed();
    KalimatLexer lxr;
    lxr.init(txt);
    lxr.tokenize();
    QVector<Token> toks = lxr.getTokens();
    int classDecl[] = {CLASS, IDENTIFIER, COLON};
    int ifStmtStart[] = { IF }, ifStmtEnd[] = {COLON};
    int forStmtStart[] = { FORALL }, forStmtEnd[] = {COLON};
    int whileStmtStart[] = { WHILE }, whileStmtEnd[] = {COLON};
    int procDeclStart[] = { PROCEDURE, IDENTIFIER, LPAREN}, procDeclEnd[] = { RPAREN, COLON};
    int funcDeclStart[] = { FUNCTION, IDENTIFIER, LPAREN}, funcDeclEnd[] = { RPAREN, COLON};

    // todo: consider adding a newline after the 'end' that teminates top-level declarations.
    if(tokensEqual(toks, classDecl, 3) ||
       tokensBeginEnd(toks, procDeclStart, procDeclEnd, 3, 2) ||
       tokensBeginEnd(toks, funcDeclStart, funcDeclEnd, 3, 2))
    {
        indentAndTerminate(li, QString::fromStdWString(L"نهاية"));
    }
    else if(tokensBeginEnd(toks, ifStmtStart, ifStmtEnd, 1,1))
    {
        indentAndTerminate(li, QString::fromStdWString(L"تم"));
    }
    else if(tokensBeginEnd(toks, forStmtStart, forStmtEnd, 1,1) ||
            tokensBeginEnd(toks, whileStmtStart, whileStmtEnd, 1,1))
    {
        indentAndTerminate(li, QString::fromStdWString(L"تابع"));
    }
    else
    {
        int n = indentOfLine(li);
        insertPlainText("\n");
        for(int i=0;i<n; i++)
            insertPlainText(" ");
    }
}
void MyEdit::indentAndTerminate(LineInfo prevLine, QString termination)
{

    int n = indentOfLine(prevLine);
    this->insertPlainText("\n");
    for(int i=0; i<4 + n; i++)
        this->insertPlainText(" ");
    this->insertPlainText("\n");
    for(int i=0; i<n; i++)
        this->insertPlainText(" ");
    this->insertPlainText(termination);
    //this->insertPlainText("\n");

    QTextCursor c = textCursor();
    c.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor, termination.length() + 1 + n);
    setTextCursor(c);
}

void MyEdit::textChangedEvent()
{
    lineTracker.setText(this->document()->toPlainText());
    lineTracker.lineColumnOfPos(this->textCursor().position(), _line, _column);
    owner->setLineIndicators(_line, _column);
}
void MyEdit::selectionChangedEvent()
{
    lineTracker.lineColumnOfPos(this->textCursor().position(), _line, _column);
    owner->setLineIndicators(_line, _column);
}

LineInfo MyEdit::currentLine()
{
    return lineTracker.line(lineTracker.lineFromPos(textCursor().position()));
}
QString MyEdit::textOfLine(LineInfo li)
{
    return this->document()->toPlainText().mid(li.start, li.length);
}
int MyEdit::indentOfLine(LineInfo li)
{
    QString line = textOfLine(li);
    int n =0;
    //todo: handle tabs
    while(n<line.length() && line[n].isSpace())
    {
        n++;
    }
    return n;
}

int MyEdit::line()
{
    return _line;
}

int MyEdit::column()
{
    return _column;
}