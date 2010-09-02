/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "codegenhelper.h"

SmallVMCodeGenerator::SmallVMCodeGenerator()
{
    Init();
}
void SmallVMCodeGenerator::Init()
{
    Output.clear();
    uniqueLabelCount = 0;
    uniqueStringConstantCount = 0;
    uniqueVariableCount = 0;
}
QString SmallVMCodeGenerator::getOutput()
{
    return Output.join("");
}

QString SmallVMCodeGenerator::makeStringConstant(QString str)
{
    if(!StringConstants.contains(str))
    {
        QString symRef = QString("%%string%1").arg(uniqueStringConstantCount++);
        StringConstants[str] = symRef;
    }
    return StringConstants[str];
}
QString SmallVMCodeGenerator::uniqueLabel()
{
    return QString("%%label%1").arg(uniqueLabelCount++);
}
QString SmallVMCodeGenerator::uniqueVariable()
{
    return QString("%%var%1").arg(uniqueVariableCount++);
}

void SmallVMCodeGenerator::gen(QString str)
{
    Output.append(str+"\n");
}

void SmallVMCodeGenerator::gen(QString str, int i)
{
    gen(QString(str).arg(i));
}

void SmallVMCodeGenerator::gen(QString str, double d)
{
    gen(QString(str).arg(d));
}
void SmallVMCodeGenerator::genWithMetaData(int metaData, QString str)
{
    gen(str+ "@"+ QString("%1").arg(metaData));
}