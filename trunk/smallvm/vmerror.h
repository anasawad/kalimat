/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef VMERROR_H
#define VMERROR_H

enum VMErrorType
{
    NoSuchVariable, NoSuchProcedure, NoSuchProcedureOrFunction,
    NoSuchField, NoSuchMethod, NoSuchExternalMethod, NoSuchEvent,
    NoSuchClass, NameDoesntIndicateAClass,

    UnrecognizedInstruction,UnrecognizedMnemonic,
    GettingFieldOnNonObject, SettingFieldOnNonObject, CallingMethodOnNonObject,
    NumericOperationOnNonNumber, LogicOperationOnNonBoolean,

    DivisionByZero,
    BuiltInOperationOnNonBuiltn,
    TypeError, InternalError, WrongNumberOfArguments, FunctionDidntReturnAValue,
    SubscribingNonArray, SubscribtMustBeInteger, SubscriptOutOfRange,
    ArgumentError, MD_IndexingWrongNumberOfDimensions,
    ElementAlreadyDefined, MethodAlreadyDefinedInClass,
    RuntimeError

};

class VMError
{
public:

    VMErrorType type;
    QStack<Frame> callStack;
    QVector<QString> args;
public:
    VMError(VMErrorType type, QStack<Frame> callStack);
    VMError(VMErrorType type);
    VMError *arg(QString s);
};

#endif // VMERROR_H
