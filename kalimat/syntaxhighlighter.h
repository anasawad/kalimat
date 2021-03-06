/**************************************************************************
** The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include "idemessages.h"
#include "../smallvm/utils.h"

class KalimatLexer;

QString removeExtraSpaces(QString input);
class SyntaxHighlighter : public QSyntaxHighlighter
{
Q_OBJECT
private:
    Translation<IdeMsg::IdeMessage> msg;
public:
     SyntaxHighlighter(QTextDocument *parent, KalimatLexer *, Translation<IdeMsg::IdeMessage> &msg);
     void highlightToHtml(QString program, QStringList &output);
     void highlightToWiki(QString program, QStringList &output);
     void highlightLiterateHtml(QString program, QStringList &output);
 protected:
    void highlightBlock(const QString &text);


 private:
     KalimatLexer *lexer;
     QMap<int, QTextCharFormat> tokenFormats;
     QTextCharFormat keywords;
     QTextCharFormat comments;
     QTextCharFormat stringLiterals;
     QTextCharFormat fileLink;

};

#endif // SYNTAXHIGHLIGHTER_H
