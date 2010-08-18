/**************************************************************************
** The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef CODEGENHELPER_H
#define CODEGENHELPER_H

#include <QString>
#include <QStringList>
#include <QMap>
class SmallVMCodeGenerator
{
    QStringList Output;
    int uniqueLabelCount;
    int uniqueStringConstantCount;
    int uniqueVariableCount;

public:
    QMap<QString, QString> StringConstants;

    SmallVMCodeGenerator();
    void Init();
    QString getOutput();

public:
    void gen(QString str);
    void gen(QString str, int i);
    void gen(QString str, double d);
    void genWithMetaData(int metaData, QString str);


    QString uniqueLabel();
    QString uniqueVariable();
    QString makeStringConstant(QString str);
};

#endif // CODEGENHELPER_H
