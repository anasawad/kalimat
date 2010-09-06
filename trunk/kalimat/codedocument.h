/**************************************************************************
** The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef CODEDOCUMENT_H
#define CODEDOCUMENT_H

#include <QTextEdit>
#include <QTabWidget>

class CodeDocument;
class DocumentContainer;
class RecentFileHandler
{
public:
    virtual void onFileTouched(QString fileName, CodeDocument *doc) = 0;
};

class CodeDocument : public QObject
{
    Q_OBJECT

private:
    bool _isDirty, isNewFile;
    QString _fileName;
    QTextEdit *editor;
    QTabWidget *tabs;
    DocumentContainer *container;
    CodeDocument(QString fileName, QTabWidget *tabs, QWidget *tabWidget, DocumentContainer *container);
public:

    static CodeDocument *newDoc(QString fileName, QTabWidget *tabs, QWidget *tabWidget, DocumentContainer *container);
    static CodeDocument *openDoc(QString fileName, QTabWidget *tabs, QWidget *tabWidget, DocumentContainer *container);
    virtual ~CodeDocument();
    QTextEdit *getEditor();
    void setDirty(bool dirty);
    bool isFileDirty();
    bool isDocNewFile();
    bool canDiscard();
    QString getFileName();
    bool doSave();
    void doSaveAs();
    void setFileName(QString);
private:
    void initTabLink(QTabWidget *tabBar, QWidget * tabWidget);
    void load();
    void save();

    bool GetSaveFilename();
    void setTabText(QString);
    QString getTabText();
public slots:
    void editor_textChanged();
    void editor_cursorPositionChanged();

};

#endif // CODEDOCUMENT_H
