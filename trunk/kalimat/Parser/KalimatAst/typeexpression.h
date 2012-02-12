#ifndef TYPEEXPRESSION_H
#define TYPEEXPRESSION_H

#include "kalimatast.h"

class TypeExpression : public KalimatAst
{
    ASTImpl _astImpl;
public:
    TypeExpression(Token pos) : _astImpl(pos) {}
    Token getPos() { return _astImpl.getPos();}
};

class TypeIdentifier : public TypeExpression
{
public:
    QString name;
    TypeIdentifier(Token pos, QString name);
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class PointerTypeExpression : public TypeExpression
{
    shared_ptr<TypeExpression> _pointeeType;
public:
    PointerTypeExpression(Token pos, shared_ptr<TypeExpression> pointeeType);
    QString toString();
    void prettyPrint(CodeFormatter *f);
    shared_ptr<TypeExpression> pointeeType() { return _pointeeType;}
};

class FunctionTypeExpression : public TypeExpression
{
    shared_ptr<TypeExpression> _retType;
    QVector<shared_ptr<TypeExpression> > _argTypes;
public:
    FunctionTypeExpression(Token pos,
                           shared_ptr<TypeExpression> retType,
                           QVector<shared_ptr<TypeExpression> > argTypes);
    QString toString();
    void prettyPrint(CodeFormatter *formatter);
    shared_ptr<TypeExpression> retType() { return _retType; }
    int argTypeCount() { return _argTypes.count(); }
    shared_ptr<TypeExpression> argType(int i) { return _argTypes[i]; }
};

#endif // TYPEEXPRESSION_H