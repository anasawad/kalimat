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
        if(at.unicode() >= L'\u0660' && at.unicode() < L'\u0669' )
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
    setRtl();
    lastInputChar = "";
    _line = _column = 0;
}

void MyEdit::setRtl()
{
    QKeyEvent ev(QKeyEvent::KeyPress, Qt::Key_Direction_R, 0, "");
    emit keyPressEvent(&ev);
    QTextOption opt = document()->defaultTextOption();
    opt.setTextDirection(Qt::RightToLeft);
    document()->setDefaultTextOption(opt);
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
        enterKeyBehavior(ev);
    }
    /*
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
    //*/
    else if(ev->text() == "," || ev->text() == arabComma)
    {
        bool rightAfterNumber = isAfterNumber(this);
        QTextEdit::keyPressEvent(ev);
        lastInputChar = ev->text();
        lastInputLine = this->line();
        if(rightAfterNumber)
        {
            QKeyEvent *otherEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_Space, 0, " ", false, 1);
            QTextEdit::keyPressEvent(otherEvent);
            lastInputChar = otherEvent->text();
            lastInputLine = this->line();
        }
    }
    else if(ev->text() == ".")
    {
        bool replace = isAfterArabicNumber(this);
        if(!replace)
            QTextEdit::keyPressEvent(ev);
        else
        {
            this->insertPlainText(QString::fromWCharArray(L"٫"));
            lastInputChar = ev->text();
            lastInputLine = this->line();
        }
    }
    else if(ev->text() == ":")
    {
        colonBehavior(ev);
        lastInputChar = ":";
        lastInputLine = this->line();
    }
    else
    {
        if(ev->text().trimmed() != "")
        {
            lastInputChar = ev->text();
            lastInputLine = this->line();
        }
        QTextEdit::keyPressEvent(ev);
    }
}

void MyEdit::tabBehavior()
{
    this->insertPlainText("    ");

}

void MyEdit::shiftTabBehavior()
{
    textCursor().beginEditBlock();
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
        int toErase = calculateDeindent(4, txt);
        if(i == lineStart)
            erasedFromA = toErase;
        if(i == lineEnd)
            erasedFromB = toErase;

        eraseFromBeginOfLine(li, toErase);
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
    textCursor().endEditBlock();
}

void MyEdit::eraseFromBeginOfLine(LineInfo li, int toErase)
{
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

int MyEdit::calculateDeindent(int by, QString lineText)
{
    int toErase = 0;
    for (int j=0; j<4; j++)
    {
        if(j==lineText.length())
            break;
        if(lineText[j]!=' ') // todo: handle real tab characters
            break;
        toErase++;
    }
    return toErase;
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

void MyEdit::enterKeyBehavior(QKeyEvent *ev)
{
    LineInfo li = currentLine();
    QString txt = textOfLine(li);
    QString lexedTxt = txt.trimmed();
    KalimatLexer lxr;
    bool tokenized = false;
    try
    {
        lxr.init(lexedTxt);
        lxr.tokenize();
        tokenized = true;
    }
    catch(UnexpectedCharException ex)
    {
    }
    catch(ColonUnsupportedInIdentifiersException ex)
    {
    }
    catch(UnexpectedEndOfFileException ex)
    {
    }
    if(!tokenized)
    {
        QTextEdit::keyPressEvent(ev);
        return;
    }
    QVector<Token> toks = lxr.getTokens();
    int classDecl[] = { CLASS, IDENTIFIER, COLON };
    int ifStmtStart[] = { IF }, ifStmtEnd[] = { COLON };
    int elsePart[] = { ELSE, COLON };
    int elseIfStart[] = { ELSE, IF }, elseIfEnd[] = { COLON };
    int forStmtStart[] = { FORALL }, forStmtEnd[] = { COLON };
    int whileStmtStart[] = { WHILE }, whileStmtEnd[] = { COLON };
    int procDeclStart[] = { PROCEDURE, IDENTIFIER, LPAREN }, procDeclEnd[] = { RPAREN, COLON };
    int funcDeclStart[] = { FUNCTION, IDENTIFIER, LPAREN }, funcDeclEnd[] = { RPAREN, COLON };
    int responseDeclStart[] = { RESPONSEOF, IDENTIFIER, IDENTIFIER }, responseDeclEnd[] = { RPAREN, COLON };
    int replyDeclStart[] = { REPLYOF, IDENTIFIER, IDENTIFIER }, replyDeclEnd[] = { RPAREN, COLON };
    int selectStmtStart[] = { SELECT }, selectStmtEnd[] = { COLON };

    int selectSendBegin[] = {SEND }, selectSendEnd[] = { COLON };
    int selectRecvBegin[] = {RECEIVE}, selectRecvEnd[] = { COLON };
    int orSendBegin[] = { OR, SEND }, orSendEnd[] = { COLON };
    int orRecvBegin[] = {OR, RECEIVE}, orRecvEnd[] = { COLON };

    QString tail = txt.right(txt.length() - column()).trimmed();
    bool endOfLine = tail == "";
    bool indented = false;
    // todo: consider adding a newline after the 'end' that teminates top-level declarations.
    textCursor().beginEditBlock();
    bool end = textCursor().atEnd();
    bool insertEnding = lastInputChar == ":" && this->line() == lastInputLine;
    if(endOfLine)
    {
        if(insertEnding && (tokensEqual(toks, classDecl, 3) ||
           tokensBeginEnd(toks, procDeclStart, procDeclEnd, 3, 2) ||
           tokensBeginEnd(toks, funcDeclStart, funcDeclEnd, 3, 2) ||
           tokensBeginEnd(toks, responseDeclStart, responseDeclEnd, 3, 2) ||
           tokensBeginEnd(toks, replyDeclStart, replyDeclEnd, 3, 2))
           )
        {
            indented = true;
            indentAndTerminate(li, QString::fromStdWString(L"نهاية"));
        }
        else if(insertEnding && tokensBeginEnd(toks, ifStmtStart, ifStmtEnd, 1,1))
        {
            indented = true;
            indentAndTerminate(li, QString::fromStdWString(L"تم"));
        }
        else if(insertEnding && tokensBeginEnd(toks, selectStmtStart, selectStmtEnd, 1,1))
        {
            indented = true;
            indentAndTerminate(li, QString::fromStdWString(L"تم"));
        }
        else if(tokensEqual(toks, elsePart, 2) ||
                tokensBeginEnd(toks, elseIfStart, elseIfEnd, 2, 1) ||
                tokensBeginEnd(toks, selectSendBegin, selectSendEnd, 1, 1) ||
                tokensBeginEnd(toks, orSendBegin, orSendEnd, 2, 1) ||
                tokensBeginEnd(toks, selectRecvBegin, selectRecvEnd, 1, 1) ||
                tokensBeginEnd(toks, orRecvBegin, orRecvEnd, 2, 1))
        {
            indented = true;
            int n = indentOfLine(li);
            insertPlainText("\n");
            for(int i=0;i<n + 4; i++)
                insertPlainText(" ");
        }
        else if(insertEnding && (tokensBeginEnd(toks, forStmtStart, forStmtEnd, 1,1) ||
                tokensBeginEnd(toks, whileStmtStart, whileStmtEnd, 1,1)))
        {
            indented = true;
            indentAndTerminate(li, QString::fromStdWString(L"تابع"));
        }
    }
    if(endOfLine && !indented)
    {
        int n = indentOfLine(li);
        insertPlainText("\n");
        for(int i=0;i<n; i++)
            insertPlainText(" ");
    }
    else if(!indented)
    {
        insertPlainText("\n");
    }

    textCursor().endEditBlock();
    if(indented)
    {
        QTextCursor c = textCursor();
        moveCursor(QTextCursor::Down);
        ensureCursorVisible();
        setTextCursor(c );
    }
    if(end)
    {
        QTextCursor c = textCursor();
        int oldpos = c.position();
        c.movePosition(QTextCursor::End);
        setTextCursor(c);
        ensureCursorVisible();
        c = textCursor();
        c.setPosition(oldpos);
        setTextCursor(c);
    }
}

void MyEdit::colonBehavior(QKeyEvent *ev)
{
    int elsePart[] = { ELSE, COLON };
    int elseIfStart[] = { ELSE, IF }, elseIfEnd[] = { COLON };

    int selectSendBegin[] = {SEND }, selectSendEnd[] = { COLON };
    int selectRecvBegin[] = {RECEIVE}, selectRecvEnd[] = { COLON };
    int orSendBegin[] = { OR, SEND }, orSendEnd[] = { COLON };
    int orRecvBegin[] = {OR, RECEIVE}, orRecvEnd[] = { COLON };

    textCursor().beginEditBlock();
    textCursor().insertText(":");
    textChangedEvent();

    try
    {
        LineInfo li = currentLine();
        QString txt = textOfLine(li);

        QString lexedTxt = txt.trimmed();
        KalimatLexer lxr;
        lxr.init(lexedTxt);
        lxr.tokenize();
        QVector<Token> toks = lxr.getTokens();

        QString tail = txt.right(txt.length() - column()).trimmed();
        bool endOfLine = tail == "";

        if(endOfLine)
        {
            if(tokensBeginEnd(toks, elseIfStart, elseIfEnd, 2, 1) ||
               tokensEqual(toks, elsePart, 2) ||
                tokensBeginEnd(toks, selectSendBegin, selectSendEnd, 1, 1) ||
                tokensBeginEnd(toks, orSendBegin, orSendEnd, 2, 1) ||
                tokensBeginEnd(toks, selectRecvBegin, selectRecvEnd, 1, 1) ||
                tokensBeginEnd(toks, orRecvBegin, orRecvEnd, 2, 1))
            {
                int toErase = calculateDeindent(4, txt);
                eraseFromBeginOfLine(li, toErase);
                QTextCursor c = textCursor();
                c.movePosition(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);
                setTextCursor(c);
            }
        }
    }
    catch(UnexpectedCharException ex)
    {
    }
    catch(ColonUnsupportedInIdentifiersException ex)
    {
    }
    catch(UnexpectedEndOfFileException ex)
    {
    }

    textCursor().endEditBlock();
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

    if(termination.length() >0)
        lastInputChar = termination[termination.length() -1];
    // For some reason moving the cursor backwards when it's at the document's end
    // and the scroll bar is visible (i.e there's hidden text above the current view)
    // scrolls to top of document (while keeping the cursor position intact)
    // so we move it only if not at the end
    // todo: fix the atEnd() case for enterKeyBehavior
    QTextCursor c = textCursor();
    c.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor, termination.length() + 1 + n);
    this->setTextCursor(c);
}

void MyEdit::deindentLine(int line, int by)
{
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

bool isSpace(QChar c)
{
    //todo: handle tabs
    return c == ' ' || c == '\t';
}

int MyEdit::indentOfLine(LineInfo li)
{
    QString line = textOfLine(li);
    int n =0;
    while(n<line.length() && isSpace(line[n]))
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
