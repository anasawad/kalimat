/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "method.h"

Method::Method(QString name,int arity)
    :Object()
{
    this->name = name;
    this->arity = arity;
    this->numReturnValues = numReturnValues;
    receiver = NULL; // 'tis a global method, not a member of a class
}

Method::Method(QString name, int arity, int numReturnValues, bool returnsReference, ValueClass *receiver)
    :Object()
{
    this->name = name;
    this->arity = arity;
    this->numReturnValues = numReturnValues;
    this->returnsReference = returnsReference;
    this->receiver = receiver;
}

void Method::Add(Instruction i)
{
    instructions.append(i);
}

void Method::Add(Instruction i, QString label)
{
    instructions.append(i);
    if(label != "")
    {
        labels[label] = instructions.count() -1;
    }
}

void Method::Add(Instruction i, QString label, int extraInfo)
{
    instructions.append(i.wExtra(extraInfo));
    if(label != "")
    {
        labels[label] = instructions.count() -1;
    }
}


Instruction &Method::Get(QString label)
{
    return instructions[labels[label]];
}
Instruction &Method::Get(int ip)
{
    return instructions[ip];
}

int Method::GetIp(QString label)
{
    return labels[label];
}
bool Method::HasInstruction(int ip)
{
    return ip < instructions.count();
}
int Method::InstructionCount()
{
    return instructions.count();
}

int Method::Arity()
{
    return arity;
}
int Method::NumReturnValues()
{
    return numReturnValues;
}
bool Method::IsReturningReference()
{
    return returnsReference;
}
QString Method::getName()
{
    return this->name;
}
