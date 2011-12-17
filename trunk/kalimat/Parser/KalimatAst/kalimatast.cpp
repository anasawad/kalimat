/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "kalimatast_incl.h"
#include "../../../smallvm/utils.h"
using namespace std;

CompilationUnit::CompilationUnit(Token pos) : _astImpl(pos)
{

}

Program::Program(Token pos, QVector<QSharedPointer<TopLevel> > elements, QVector<QsharedPointer<StrLiteral> > usedModules, QVector<QSharedPointer<TopLevel> > originalElements)
    : CompilationUnit(pos),
      _elements(elements),
      _usedModules(usedModules),
      _originalElements(originalElements)
{
    /*
    for(int i=0; i<elements.count(); i++)
        _elements.append(QSharedPointer<TopLevel>(elements[i]));

    for(int i=0; i<usedModules.count(); i++)
        _usedModules.append(QSharedPointer<StrLiteral>(usedModules[i]));
    for(int i=0; i<originalElements.count(); i++)
        _originalElements.append(originalElements[i]);
        */
}

QString Program::toString()
{
    QStringList ret;
    for(int i=0; i<elementCount(); i++)
    {
        ret.append(element(i)->toString());
    }
    return _ws(L"برنامج(").append(ret.join(_ws(L"، "))).append(")");
}

Module::Module(Token pos,
               QSharedPointer<Identifier> name,
               QVector<QSharedPointer<Declaration> > module,
               QVector<QSharedPointer<StrLiteral> > usedModules)
    :CompilationUnit(pos),
    _name(name),
      _declarations(module),
      _usedModules(usedModules)
{
    /*
    for(int i=0; i<module.count(); i++)
        _declarations.append(QSharedPointer<Declaration>(module[i]));

    for(int i=0; i<usedModules.count(); i++)
        _usedModules.append(QSharedPointer<StrLiteral>(usedModules[i]));
    //*/
}

Module::~Module()
{

}

QString Module::toString()
{
    QStringList ret;
    for(int i=0; i<declCount(); i++)
    {
        ret.append(decl(i)->toString());
    }
    return _ws(L"وحدة(").append(ret.join(_ws(L"، "))).append(")");
}

TopLevel::TopLevel(Token pos)
    :_astImpl(pos)
{

}

Statement::Statement(Token pos): TopLevel(pos)
{

}

Expression::Expression(Token pos) : _astImpl(pos)
{

}

AssignableExpression::AssignableExpression(Token pos) : Expression(pos)
{

}

Declaration::Declaration(Token pos, bool isPublic) : TopLevel(pos)
{
    _isPublic = isPublic;
}

bool Declaration::isPublic()
{
    return _isPublic;
}

IOStatement::IOStatement(Token pos) : Statement(pos)
{
}

GraphicsStatement::GraphicsStatement(Token pos) : Statement(pos)
{

}

AssignmentStmt::AssignmentStmt(Token pos,
                               QSharedPointer<AssignableExpression> _variable,
                               QSharedPointer<Expression> _value)
    :Statement(pos),
    _variable(_variable),
    _value(_value)
{
}

QString AssignmentStmt::toString()
{
    return _ws(L"=(%1، %2)").arg(variable()->toString(),value()->toString());
}

IfStmt::IfStmt(Token pos,
               QSharedPointer<Expression> _condition,
               QSharedPointer<Statement> _thenPart,
               QSharedPointer<Statement> _elsePart)
    :Statement(pos),
    _condition(_condition),
    _thenPart(_thenPart),
    _elsePart(_elsePart)
{

}

QString IfStmt::toString()
{
    if(elsePart() == NULL)
        return _ws(L"إذا(%1، %2)").arg(condition()->toString())
                                   .arg(thenPart()->toString());
    else
        return _ws(L"إذا(%1، %2، %3)").arg(condition()->toString())
                                      .arg(thenPart()->toString())
                                      .arg(elsePart()->toString());
}

WhileStmt::WhileStmt(Token pos,
                     QSharedPointer<Expression> _condition,
                     QSharedPointer<Statement> _statement)
    :Statement(pos),
    _condition(condition),
    _statement(statement)
{
}

QString WhileStmt::toString()
{

    return _ws(L"طالما(%1، %2)").arg(condition()->toString())
                               .arg(statement()->toString());
}

ForAllStmt::ForAllStmt(Token pos,
                       QSharedPointer<Identifier> variable,
                       QSharedPointer<Expression> from,
                       QSharedPointer<Expression> to,
                       QSharedPointer<Statement>  statement)
    :Statement(pos),
    _variable(variable),
    _from(from),
    _to(to),
    _statement(statement)
{
}

QVector<Identifier *> ForAllStmt::getIntroducedVariables()
{
    QVector<Identifier *> ret;
    ret.append(variable());
    return ret;
}

QString ForAllStmt::toString()
{

    return _ws(L"لكل(%1، %2، %3، %4)")
            .arg(variable()->toString())
            .arg(from()->toString())
            .arg(to()->toString())
            .arg(statement()->toString());
}

ReturnStmt::ReturnStmt(Token pos ,QSharedPointer<Expression> returnVal)
    :Statement(pos),
    _returnVal(returnVal)
{
}

QString ReturnStmt::toString()
{
    return _ws(L"ارجع(%1)").arg(returnVal()->toString());
}

DelegationStmt::DelegationStmt(Token pos, QSharedPointer<IInvokation> invokation)
    :Statement(pos),
    _invokation(invokation)
{
}

QString DelegationStmt::toString()
{
    return _ws(L"وكل إلى(%1)").arg(invokation()->toString());
}

LaunchStmt::LaunchStmt(Token pos, QSharedPointer<IInvokation> invokation)
    :Statement(pos),
    _invokation(invokation)
{
}

QString LaunchStmt::toString()
{
    return _ws(L"شغل(%1)").arg(invokation()->toString());
}


LabelStmt::LabelStmt(Token pos, QSharedPointer<Expression> target)
    : Statement(pos),
      _target(target)
{

}
QString LabelStmt::toString()
{
    return _ws(L"علامة(%1)").arg(target()->toString());
}

GotoStmt::GotoStmt(Token pos,
                   bool _targetIsNumber,
                   QSharedPointer<Expression> target)
    :Statement(pos)
{
    targetIsNumber = _targetIsNumber;
    if(targetIsNumber)
        _numericTarget = target.dynamicCast<NumLiteral>();
    else
        _idTarget = target.dynamicCast<Identifier>();
}

QString GotoStmt::toString()
{
    return _ws(L"اذهب(%1)").arg(targetIsNumber? _numericTarget.data()->toString(): _idTarget.data()->toString());
}

PrintStmt::PrintStmt(Token pos,
                     QSharedPointer<Expression> fileObject,
                     QVector<QSharedPointer<Expression> > args,
                     QVector<QSharedPointer<Expression> > widths,
                     bool printOnSameLine)
    :IOStatement(pos),
     _fileObject(fileObject),
     _args(args),
     _widths(widths),
     printOnSameLine(printOnSameLine)
{
    /*
    for(int i= 0; i<args.count();i++)
    {
        this->_args.append(QSharedPointer<Expression>(args[i]));
        this->_widths.append(QSharedPointer<Expression>(widths[i]));
    }
    this->printOnSameLine = printOnSameLine;
    */
}
QString PrintStmt::toString()
{
    QStringList ret;
    for(int i=0; i<argCount(); i++)
    {
        ret.append(arg(i)->toString());
    }
    return _ws(L"اطبع(").append(ret.join(_ws(L"، "))).append(")");
}

ReadStmt::ReadStmt(Token pos,
                   QSharedPointer<Expression> fileObject,
                   QString prompt,
                   const QVector<QSharedPointer<AssignableExpression> >&variables,
                   QVector<bool> readNumberFlags)
    :IOStatement(pos),
     _fileObject(fileObject),
      prompt(prompt),
      _variables(variables),
      readNumberFlags(readNumberFlags)
{
    /*
    this->prompt = prompt;
    for(int i=0; i<variables.count(); i++)
        this->_variables.append(QSharedPointer<AssignableExpression>(variables[i]));
    this->readNumberFlags = readNumberFlags;
    */
}

QString ReadStmt::toString()
{
    QStringList ret;
    for(int i=0; i<variableCount(); i++)
    {
        ret.append(variable(i)->toString());
    }
    return _ws(L"اقرأ(\"").append(prompt).append("\"،").append(ret.join(_ws(L"، "))).append("}");
}

DrawPixelStmt::DrawPixelStmt(Token pos,
                             QSharedPointer<Expression>x,
                             QSharedPointer<Expression> y,
                             QSharedPointer<Expression> color)
    :GraphicsStatement(pos),
    _x(x),
    _y(y),
    _color(color)
{
}

QString DrawPixelStmt::toString()
{
    return _ws(L"رسم.نقطة(%1، %2، %3)")
            .arg(x()->toString())
            .arg(y()->toString())
            .arg(color()? color()->toString(): "default");
}

DrawLineStmt::DrawLineStmt(Token pos,
                           QSharedPointer<Expression> x1,
                           QSharedPointer<Expression> y1,
                           QSharedPointer<Expression> x2,
                           QSharedPointer<Expression> y2,
                           QSharedPointer<Expression> color)
        :GraphicsStatement(pos),
         _x1(x1),
         _y1(y1),
         _x2(x2),
         _y2(y2),
         _color(color)
{
}

QString DrawLineStmt::toString()
{
    return _ws(L"رسم.خط([%1، %2]، [%3، %4]، %5)")
            .arg(x1()? x1()->toString(): "current")
            .arg(y1()? y1()->toString(): "current")
            .arg(x2()->toString())
            .arg(y2()->toString())
            .arg(color()? color()->toString(): "default");
}

DrawRectStmt::DrawRectStmt(Token pos,
                           QSharedPointer<Expression> x1,
                           QSharedPointer<Expression> y1,
                           QSharedPointer<Expression> x2,
                           QSharedPointer<Expression> y2,
                           QSharedPointer<Expression> color,
                           QSharedPointer<Expression> filled)
        :GraphicsStatement(pos),
        _x1(x1),
        _y1(y1),
        _x2(x2),
        _y2(y2),
        _color(color),
        _filled(filled)

{
}

QString DrawRectStmt::toString()
{
    return _ws(L"رسم.مستطيل([%1، %2]، [%3، %4],%5، %6)")
            .arg(x1()? x1()->toString() : "current")
            .arg(y1()? y1()->toString() : "current")
            .arg(x2()->toString())
            .arg(y2()->toString())
            .arg(color()? color()->toString(): "default")
            .arg(filled()?filled()->toString(): _ws(L"خطأ"));
}

DrawCircleStmt::DrawCircleStmt(Token pos,
                               QSharedPointer<Expression> cx,
                               QSharedPointer<Expression> cy,
                               QSharedPointer<Expression> radius,
                               QSharedPointer<Expression> color,
                               QSharedPointer<Expression> filled)
        :GraphicsStatement(pos),
        _cx(cx),
        _cy(cy),
        _radius(radius),
        _color(color),
        _filled(filled)

{
}

QString DrawCircleStmt::toString()
{
    return _ws(L"رسم.دائرة([%1، %2]، %3، %4، %5)")
            .arg(cx()->toString())
            .arg(cy()->toString())
            .arg(radius()->toString())
            .arg(color()? color()->toString(): "default")
            .arg((filled()?filled()->toString(): _ws(L"خطأ")));
}

DrawSpriteStmt::DrawSpriteStmt(Token pos,
                               QSharedPointer<Expression> x,
                               QSharedPointer<Expression> y,
                               QSharedPointer<Expression> sprite)
        :GraphicsStatement(pos),
        _x(x),
        _y(y),
        _sprite(sprite)

{
}

QString DrawSpriteStmt::toString()
{
    return _ws(L"رسم.طيف(%، [%2، %3])")
            .arg(sprite()->toString())
            .arg(x()->toString())
            .arg(y()->toString());
}

ZoomStmt::ZoomStmt(Token pos,
                   QSharedPointer<Expression> x1,
                   QSharedPointer<Expression> y1,
                   QSharedPointer<Expression> x2,
                   QSharedPointer<Expression> y2)
        :GraphicsStatement(pos),
        _x1(x1),
        _y1(y1),
        _x2(x2),
        _y2(y2)
{
}
QString ZoomStmt::toString()
{
    return _ws(L"زووم([%1,%2],[%3,%4])")
            .arg(x1()->toString())
            .arg(y1()->toString())
            .arg(x2()->toString())
            .arg(y2()->toString());
}

EventStatement::EventStatement(Token pos ,EventType type, QSharedPointer<Identifier> handler)
        :Statement(pos),
         type(type),
         _handler(handler)

{
}

QString EventStatement::toString()
{
    return _ws(L"عند(%1، %2)").arg(type).arg(handler()->toString());
}

ChannelCommunicationStmt::ChannelCommunicationStmt(Token pos)
    : Statement(pos)
{

}

SendStmt::SendStmt(Token pos,
                   bool signal,
                   QSharedPointer<Expression> value,
                   QSharedPointer<Expression> channel)
    : ChannelCommunicationStmt(pos), signal(signal), _value(value), _channel(channel)
{
}

QString SendStmt::toString()
{
    return _ws(L"ارسل(%1، %2)").arg(signal? _ws(L"إشارة") : value()->toString(), channel()->toString());
}

ReceiveStmt::ReceiveStmt(Token pos,
                         bool signal,
                         QSharedPointer<AssignableExpression> value,
                         QSharedPointer<Expression> channel)
    : ChannelCommunicationStmt(pos), signal(signal), _value(value), _channel(channel)
{
}

QString ReceiveStmt::toString()
{
    return _ws(L"تسلم(%1، %2)").arg(signal? _ws(L"إشارة") : value()->toString(), channel()->toString());
}

SelectStmt::SelectStmt(Token pos,
                       QVector<QSharedPointer<ChannelCommunicationStmt> > conditions,
                       QVector<QSharedPointer<Statement > > actions)
    : Statement(pos),
      _conditions(conditions),
      _actions(actions)
{
   /*
    for(int i=0; i<conditions.count();i++)
        _conditions.append(QSharedPointer<ChannelCommunicationStmt>(conditions[i]));
    for(int i=0; i<actions.count();i++)
        _actions.append(QSharedPointer<Statement>(actions[i]));
    */
}

QString SelectStmt::toString()
{
    QStringList condact;
    for(int i=0; i<count(); i++)
    {
        condact.append(QString("%1 => %2")
                       .arg(condition(i)->toString())
                       .arg(action(i)->toString()));
    }
    return _ws(L"تخير({%1})").arg(condact.join(", "));
}

BlockStmt::BlockStmt(Token pos, QVector<Statement *> statements)
        :Statement(pos),
          _statements(statements)

{
    //for(int i=0; i<statements.count(); i++)
    //    _statements.append(QSharedPointer<Statement>(statements[i]));
}
QVector<Statement *> BlockStmt::getStatements()
{
    QVector<Statement *> ret;
    for(int i=0; i<statementCount(); i++)
    {
        ret.append(statement(i));
    }
    return ret;
}

QString BlockStmt::toString()
{
    QStringList ret;
    for(int i=0; i<statementCount(); i++)
    {
        ret.append(statement(i)->toString());
    }
    return QString("{ ").append(ret.join(_ws(L"، "))).append(" }");
}

InvokationStmt::InvokationStmt(Token pos ,QSharedPointer<Expression> expression)
        :Statement(pos),
        _expression(expression)
{
}
QString InvokationStmt::toString()
{
    return expression()->toString();
}

Identifier::Identifier(Token pos ,QString name)
        :AssignableExpression(pos)
{
    this->name = name;
}
QString Identifier::toString()
{
    return name;
}

TypeIdentifier::TypeIdentifier(Token pos ,QString name)
        :TypeExpression(pos)
{
    this->name = name;
}
QString TypeIdentifier::toString()
{
    return name;
}

PointerTypeExpression::PointerTypeExpression(Token pos, TypeExpression *pointeeType)
    :TypeExpression(pos), _pointeeType(pointeeType)
{
}

QString PointerTypeExpression::toString()
{
    return QString("pointer(%1)").arg(pointeeType()->toString());
}

FunctionTypeExpression::FunctionTypeExpression(Token pos, TypeExpression *retType, QVector<TypeExpression *> argTypes)
    :TypeExpression(pos), _retType(retType)
{
    for(int i=0; i<argTypes.count(); i++)
        _argTypes.append(QSharedPointer<TypeExpression>(argTypes[i]));
}
QString FunctionTypeExpression::toString()
{
    QStringList strs;
    for(int i=0; i<argTypeCount(); i++)
        strs.append(argType(i)->toString());
    return QString("func(%1,[%2])").arg(retType()?retType()->toString():"void",strs.join(", "));
}

Pattern::Pattern(Token pos)
    : _astImpl(pos)
{

}

QString VarPattern::toString()
{
    return id()->toString();
}

ArrayPattern::ArrayPattern(Token pos, QVector<Pattern *> elements)
    : Pattern(pos)
{
    for(QVector<Pattern *>::const_iterator i=elements.begin(); i != elements.end(); ++i)
    {
        _elements.append(QSharedPointer<Pattern>(*i));
    }
}

QString ArrayPattern::toString()
{
    QStringList items;
    for(int i=0; i<elementCount(); i++)
        items.append(element(i)->toString());
    return _ws(L"نمط.مصفوفة([%1 %2])").arg(items.join(", ")).arg(this->fixedLength? "": "...");
}

ObjPattern::ObjPattern(Token pos,
                       Identifier *classId,
                       QVector<Identifier *> fieldNames,
                       QVector<Pattern *> fieldPatterns)
    : Pattern(pos), _classId(classId)
{
    for(QVector<Identifier *>::const_iterator i=fieldNames.begin(); i != fieldNames.end(); ++i)
    {
        _fieldNames.append(QSharedPointer<Identifier>(*i));
    }

    for(QVector<Pattern *>::const_iterator i=fieldPatterns.begin(); i != fieldPatterns.end(); ++i)
    {
        _fieldPatterns.append(QSharedPointer<Pattern>(*i));
    }
}

QString ObjPattern::toString()
{
    QStringList pairs;
    for(int i=0;i<this->fieldCount(); i++)
    {
        pairs.append(QString("%1=%2").arg(fieldName(i)->name).arg(fieldPattern(i)->toString()));
    }
    return QString(_ws(L"نمط.كائني(%1،%2)")).arg(this->classId()->name).arg(pairs.join(", "));
}

MapPattern::MapPattern(Token pos,
                       QVector<Expression *> keys,
                       QVector<Pattern *> values)
    : Pattern(pos)
{
    for(QVector<Expression*>::const_iterator i=keys.begin(); i != keys.end(); ++i)
    {
        _keys.append(QSharedPointer<Expression>(*i));
    }

    for(QVector<Pattern *>::const_iterator i=values.begin(); i != values.end(); ++i)
    {
        _values.append(QSharedPointer<Pattern>(*i));
    }
}

QString MapPattern::toString()
{
    QStringList pairs;
    for(int i=0;i<this->pairCount(); i++)
    {
        pairs.append(QString("%1=>%2").arg(key(i)->toString()).arg(value(i)->toString()));
    }
    return QString(_ws(L"نمط.قاموسي(%1)"))
            .arg(pairs.join(", "));
}

BinaryOperation::BinaryOperation(Token pos ,QString op, Expression *op1, Expression *op2)
    : Expression(pos),
    _operator(op),
    _operand1(op1),
    _operand2(op2)

{
}

QString BinaryOperation::toString()
{
    return _ws(L"عملية(%1، %2، %3)")
            .arg(_operator)
            .arg(operand1()->toString())
            .arg(operand2()->toString());
}

IsaOperation::IsaOperation(Token pos, Expression *expression, Identifier *type)
    : Expression(pos),
      _expression(expression),
      _type(type)
{

}

QString IsaOperation::toString()
{
    return _ws(L"%1 هو %2").arg(expression()->toString()).arg(type()->toString());
}

MatchOperation::MatchOperation(Token pos,
                               Expression *expression,
                               Pattern *pattern)
    : Expression(pos),
      _expression(expression),
      _pattern(pattern)
{

}

QString MatchOperation::toString()
{
    return _ws(L"%1 ~ %2").arg(expression()->toString()).arg(pattern()->toString());
}

UnaryOperation::UnaryOperation(Token pos ,QString __operator, Expression *operand)
    : Expression(pos),
    _operator(__operator),
    _operand(operand)
{
}
QString UnaryOperation::toString()
{
    return _ws(L"عملية(%1، %2)")
            .arg(_operator)
            .arg(operand()->toString());
}


NumLiteral::NumLiteral(Token pos ,QString lexeme) : SimpleLiteral(pos)
{
    bool ok;
    QLocale loc(QLocale::Arabic, QLocale::Egypt);
    longNotDouble = true;
    lValue= loc.toLongLong(lexeme, &ok, 10);
    if(!ok)
        lValue = lexeme.toLong(&ok, 10);

    if(!ok)
    {
        longNotDouble = false;
        dValue = loc.toDouble(lexeme, &ok);
    }
    if(!ok)
        dValue = lexeme.toDouble(&ok);

    valueRecognized = ok;

}
QString NumLiteral::toString()
{
    return _ws(L"عدد(%1)")
            .arg(repr());
}

QString NumLiteral::repr()
{
    return QString("%1").arg(longNotDouble? lValue: dValue);
}

StrLiteral::StrLiteral(Token pos ,QString value) : SimpleLiteral(pos)
{
    this->value = value;
}
QString StrLiteral::toString()
{
    return _ws(L"نص(%1)")
            .arg(repr());
}

QString strLiteralRepr(QString value)
{
    QStringList ret;
    ret.append("\"");
    for(int i=0; i<value.length(); i++)
    {
        QChar c = value[i];
        if(c =='"')
            ret.append("\"\"");
        else
            ret.append(c);
    }
    ret.append("\"");
    return ret.join("");
}

QString StrLiteral::repr()
{
    return strLiteralRepr(this->value);
}

NullLiteral::NullLiteral(Token pos) : SimpleLiteral(pos)
{

}
QString NullLiteral::toString()
{
    return repr();
}

QString NullLiteral::repr()
{
    return _ws(L"لاشيء");
}

BoolLiteral::BoolLiteral(Token pos, bool _value) : SimpleLiteral(pos)
{
    value = _value;
}
QString BoolLiteral::toString()
{
    return repr();
}

QString BoolLiteral::repr()
{
    return value? QString::fromWCharArray(L"صحيح") : QString::fromWCharArray(L"خطأ");
}

ArrayLiteral::ArrayLiteral(Token pos, QVector<Expression *>data)
    : Literal(pos)
{
    for(int i=0; i<data.count(); i++)
        _data.append(QSharedPointer<Expression>(data[i]));
}

QString ArrayLiteral::toString()
{
    return _ws(L"مصفوفة(%1)").arg(vector_toString(_data));
}

MapLiteral::MapLiteral(Token pos, QVector<Expression *>data)
    : Literal(pos)
{
    for(int i=0; i<data.count(); i++)
        _data.append(QSharedPointer<Expression>(data[i]));
}

QString MapLiteral::toString()
{
    return _ws(L"قاموس(%1)").arg(vector_toString(_data));
}

IInvokation::IInvokation(Token pos)
    :Expression(pos)
{

}

Invokation::Invokation(Token pos ,Expression *functor, QVector<Expression *>arguments)
    : IInvokation(pos),
    _functor(functor)
{
    for(int i=0; i<arguments.count(); i++)
        _arguments.append(QSharedPointer<Expression>(arguments[i]));
}

QString Invokation::toString()
{
    return _ws(L"نداء(%1، %2)").arg(functor()->toString()).arg(vector_toString(_arguments));
}

MethodInvokation::MethodInvokation(Token pos ,Expression *receiver, Identifier *methodSelector, QVector<Expression *>arguments)
    :IInvokation(pos),
    _receiver(receiver),
    _methodSelector(methodSelector)
{
    for(int i=0; i<arguments.count(); i++)
        _arguments.append(QSharedPointer<Expression>(arguments[i]));
}

QString MethodInvokation::toString()
{
    return _ws(L"نداء.وسيلة(%1، %2، %3)")
            .arg(receiver()->toString())
            .arg(methodSelector()->toString())
            .arg(vector_toString(_arguments));
}

Idafa::Idafa(Token pos ,Identifier *modaf, Expression *modaf_elaih)
    :AssignableExpression(pos),
    _modaf(modaf),
    _modaf_elaih(modaf_elaih)
{
}

QString Idafa::toString()
{
    return _ws(L"اضافة(%1، %2").arg(modaf()->toString(), modaf_elaih()->toString());
}

ArrayIndex::ArrayIndex(Token pos ,Expression *array, Expression *index)
    :AssignableExpression(pos),
    _array(array),
    _index(index)
{
}

QString ArrayIndex::toString()
{
    return QString("%1[%2]").arg(array()->toString(), index()->toString());
}

MultiDimensionalArrayIndex::MultiDimensionalArrayIndex(Token pos, Expression *array, QVector<Expression *> indexes)
    :AssignableExpression(pos),
    _array(array)
{
    for(int i=0; i<indexes.count(); i++)
        _indexes.append(QSharedPointer<Expression>(indexes[i]));
}

QString MultiDimensionalArrayIndex::toString()
{
    return QString("%1[%2]").arg(array()->toString(), vector_toString(_indexes));
}

ObjectCreation::ObjectCreation(Token pos ,Identifier *className)
    :Expression(pos),
    _className(className)
{
}

QString ObjectCreation::toString()
{
    return QString("جديد(%1)").arg(className()->name);
}

ProceduralDecl::ProceduralDecl(Token pos,
                               Token endingToken,
                               QSharedPointer<Identifier> procName,
                               QVector<QSharedPointer<Identifier> > formals,
                               QSharedPointer<BlockStmt> body,
                               bool isPublic)
    :Declaration(pos, isPublic),
    _procName(procName),
    _formals(formals),
    _body(body),
    _endingToken(endingToken)
{
    /*
    for(int i=0; i<formals.count(); i++)
        _formals.append(QSharedPointer<Identifier>(formals[i]));
    */
}

QVector<QSharedPointer<Identifier> > ProceduralDecl::getIntroducedVariables()
{
    return _formals;
}

ProcedureDecl::ProcedureDecl(Token pos,
                             Token endingToken,
                             QSharedPointer<Identifier> procName,
                             QVector<QSharedPointer<Identifier> >formals,
                             QSharedPointer<BlockStmt> body,
                             bool isPublic)
    :ProceduralDecl(pos, endingToken, procName, formals, body, isPublic)
{

}

QString ProcedureDecl::toString()
{
    QString pname = procName()->toString();
    // todo: Rename the actual 'main' procedure
    if(pname=="%main")
        pname = _ws(L"<البداية>");

    return _ws(L"إجراء(%1، %2، %3)")
            .arg(procName()->toString())
            .arg(vector_toString(_formals))
            .arg(body()->toString());
}

FunctionDecl::FunctionDecl(Token pos,
                           Token endingToken,
                           QSharedPointer<Identifier> procName,
                           QVector<QSharedPointer<Identifier> > formals,
                           QSharedPointer<BlockStmt> body,
                           bool isPublic)
    :ProceduralDecl(pos, endingToken, procName, formals, body, isPublic)
{

}

QString FunctionDecl::toString()
{
    return _ws(L"دالة(%1، %2، %3)")
            .arg(procName()->toString())
            .arg(vector_toString(_formals))
            .arg(body()->toString());
}

MethodInfo::MethodInfo(int arity, bool isFunction)
{
    this->arity = arity;
    this->isFunction = isFunction;
}

// Degenerate constructor, create only to allow usage in QMap<>...etc
MethodInfo::MethodInfo()
{
    arity = -1;
    isFunction = false;
}

FFILibraryDecl::FFILibraryDecl(Token pos, QString libName, QVector<QSharedPointer<Declaration> > decls, bool isPublic)
    : Declaration(pos, isPublic), libName(libName),_decls(decls)
{
}

QString FFILibraryDecl::toString()
{
    QStringList strs;
    for(int i=0; i<declCount(); i++)
        strs.append(decl(i)->toString());
    return QString("library(%1, %2)").arg(libName).arg(strs.join(", "));
}

FFIProceduralDecl::FFIProceduralDecl(Token pos,
                                     bool isFunctionNotProc,
                                     TypeExpression *theReturnType,
                                     QVector<TypeExpression *> paramTypes,
                                     QString procName,
                                     QString symbol,
                                     bool isPublic)
    : Declaration(pos, isPublic), _returnType(theReturnType)
{
    this->isFunctionNotProc = isFunctionNotProc;
    this->procName = procName;
    this->symbol = symbol;
    for(int i=0; i<paramTypes.count(); i++)
        _paramTypes.append(QSharedPointer<TypeExpression>(paramTypes[i]));
}

QString FFIProceduralDecl::toString()
{
    QStringList str1;
    for(int i=0; i<paramTypeCount(); i++)
        str1.append(paramType(i)->toString());
    return QString ("ffi(%1,%2,%3,[%4])").arg(procName).arg(symbol)
            .arg(returnType()?returnType()->toString():"void").arg(str1.join(", "));
}

FFIStructDecl::FFIStructDecl(Token pos,
                             Identifier *name,
                             QVector<Identifier *> fieldNames,
                             QVector<TypeExpression *> fieldTypes,
                             QVector<int> fieldBatches,
                             bool isPublic)
    : Declaration(pos, isPublic), _name(name)
{
    for(int i=0; i<fieldNames.count(); i++)
    {
        _fieldNames.append(QSharedPointer<Identifier>(fieldNames[i]));
        _fieldTypes.append(QSharedPointer<TypeExpression>(fieldTypes[i]));
    }
    this->fieldBatches = fieldBatches;
}

QString FFIStructDecl::toString()
{
    QStringList strs;
    for(int i=0; i<fieldCount(); i++)
    {
        strs.append(QString("%1:%2").arg(fieldName(i)->toString()).arg(fieldType(i)->toString()));
    }
    return QString("ffiStruct(%1,%2)").arg(name()->toString()).arg(strs.join(", "));
}

ClassDecl::ClassDecl(Token pos,
                     Identifier *name,
                     QVector<Identifier*>fields,
                     QMap<QString, MethodInfo> methodPrototypes,
                     QVector<QSharedPointer<ClassInternalDecl> > internalDecls,
                     QMap<QString, TypeExpression *> fieldMarshalAs,
                     bool isPublic)
        :Declaration(pos, isPublic),
        _name(name),
        _methodPrototypes(methodPrototypes),
        _internalDecls(internalDecls),
        _ancestorName(NULL),
        _ancestorClass(NULL)
{
    for(int i=0; i<fields.count(); i++)
    {
        _fields.append(QSharedPointer<Identifier>(fields[i]));
    }
    for(QMap<QString, TypeExpression *>::const_iterator i= fieldMarshalAs.begin(); i != fieldMarshalAs.end(); ++i)
    {
        _fieldMarshallAs[i.key()] = QSharedPointer<TypeExpression>(i.value());
    }
}

ClassDecl::ClassDecl(Token pos,
                     Identifier *ancestorName,
                     Identifier *name,
                     QVector<Identifier*>fields,
                     QMap<QString, MethodInfo> methodPrototypes,
                     QVector<QSharedPointer<ClassInternalDecl> > internalDecls,
                     QMap<QString, TypeExpression *> fieldMarshalAs,
                     bool isPublic)
        :Declaration(pos, isPublic),
        _name(name),
        _methodPrototypes(methodPrototypes),
        _internalDecls(internalDecls),
        _ancestorName(ancestorName),
        _ancestorClass(NULL)
{
    for(int i=0; i<fields.count(); i++)
        _fields.append(QSharedPointer<Identifier>(fields[i]));
    for(QMap<QString, TypeExpression *>::const_iterator i= fieldMarshalAs.begin(); i != fieldMarshalAs.end(); ++i)
    {
        _fieldMarshallAs[i.key()] = QSharedPointer<TypeExpression>(i.value());
    }
}

QString ClassDecl::toString()
{
    QMap<QString, MethodInfo> &m = this->_methodPrototypes;
    QStringList lst;
    for(int i=0; i<m.keys().count(); i++)
    {
        lst.append(QString("%1/%2").arg(m.keys().at(i))
                   .arg(m[m.keys().at(i)].arity));
    }
    QString map = lst.join(",");

    return _ws(L"فصيلة(%1، %2، %3")
            .arg(name()->toString())
            .arg(vector_toString(_fields))
            .arg(map);

}

void ClassDecl::insertMethod(QString name, QSharedPointer<MethodDecl>m)
{
    _methods[name] = m;
}

bool ClassDecl::containsMethod(QString name)
{
    if(_methods.contains(name))
        return true;
    if(!_ancestorClass.isNull() && _ancestorClass.data()->containsMethod(name))
        return true;
    return false;
}

MethodDecl *ClassDecl::method(QString name)
{
    if(_methods.contains(name))
        return _methods[name].data();
    if(!_ancestorClass.isNull())
        return _ancestorClass.data()->method(name);
    return NULL;
}

bool ClassDecl::containsPrototype(QString name)
{
    if(_methodPrototypes.contains(name))
        return true;
    if(!_ancestorClass.isNull() && _ancestorClass.data()->containsPrototype(name))
        return true;
    return false;
}

MethodInfo ClassDecl::methodPrototype(QString name)
{
    if(_methodPrototypes.contains(name))
        return _methodPrototypes[name];
    return _ancestorClass.data()->methodPrototype(name);
}

void ClassDecl::setAncestorClass(QSharedPointer<ClassDecl> cd)
{
    _ancestorClass = cd;
}

GlobalDecl::GlobalDecl(Token pos ,QString varName, bool isPublic)
    :Declaration(pos, isPublic)
{
    this->varName = varName;
}
QString GlobalDecl::toString()
{
    return _ws(L"مشترك(%1)").arg(varName);
}

MethodDecl::MethodDecl(Token pos, Token endingToken, Identifier *className, Identifier *receiverName, Identifier *methodName
                       , QVector<Identifier *>formals, BlockStmt *body, bool isFunctionNotProcedure)

       :ProceduralDecl(pos, endingToken, methodName, formals, body, true),
       _className(className),
       _receiverName(receiverName)
{
    this->isFunctionNotProcedure = isFunctionNotProcedure;
    this->_formals.prepend(QSharedPointer<Identifier>(receiverName));
}

QString MethodDecl::toString()
{
    return QString("%1(%2,%3,%4)")
            .arg(isFunctionNotProcedure?_ws(L"رد")
                :
                _ws(L"استجابة"))
            .arg(procName()->toString())
            .arg(vector_toString(_formals))
            .arg(body()->toString());
}
// Pretty printing
void Program::prettyPrint(CodeFormatter *f)
{
    for(int i=0; i<_originalElements.count(); i++)
    {
        TopLevel *el = _originalElements[i];
        // todo: loss of information
        if(el->attachedComments.trimmed() != "")
        {
            //f->print("--");
            f->print(el->attachedComments);
        }
        el->prettyPrint(f);
        f->nl();
    }
}

void Module::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"وحدة");
    this->name()->prettyPrint(f);
    f->nl();

    for(int i=0; i<declCount(); i++)
    {
        decl(i)->prettyPrint(f);
        f->nl();
        f->nl();
    }
}

void AssignmentStmt::prettyPrint(CodeFormatter *f)
{
    variable()->prettyPrint(f);
    f->space();
    f->print("=");
    f->space();
    value()->prettyPrint(f);
}

void IfStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"إذا");
    condition()->prettyPrint(f);
    f->space();
    f->colon();

    BlockStmt *thnBlk = dynamic_cast<BlockStmt *>(this->thenPart());
    if(thnBlk)
        f->nl();
    this->thenPart()->prettyPrint(f);

    if(this->elsePart())
    {
        BlockStmt *elsBlk = dynamic_cast<BlockStmt *>(this->elsePart());
        if(elsBlk)
        {
            f->printKw(L"وإلا");
            f->colon();
            f->nl();
            elsBlk->prettyPrint(f);
//            f->nl();
            f->printKw(L"تم");
        }
        else
        {
            f->space();
            f->printKw(L"وإلا");
            IfStmt *elseItselfIsAnotherIf = dynamic_cast<IfStmt *>(this->elsePart());
            if(!elseItselfIsAnotherIf)
            {
                f->colon();
                f->space();
            }
            this->elsePart()->prettyPrint(f);
        }
    }
    else if(thnBlk)
    {
        f->printKw(L"تم");
        f->nl(); // for extra neatness, add an empty line after block statements
    }
}

void WhileStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"طالما");
    condition()->prettyPrint(f);
    //f->space();
    f->colon();

    BlockStmt *actionBlk = dynamic_cast<BlockStmt *>(this->statement());
    if(actionBlk)
        f->nl();
    else
        f->space();

    this->statement()->prettyPrint(f);
    if(!actionBlk)
        f->space();
    f->printKw(L"تابع");
    if(actionBlk)
        f->nl(); // for extra neatness, add an empty line after block statements
}

void ForAllStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"لكل");
    this->variable()->prettyPrint(f);
    f->space();
    f->printKw(L"من");
    this->from()->prettyPrint(f);
    f->space();
    f->printKw(L"إلى");
    this->to()->prettyPrint(f);
    //f->space();
    f->colon();

    BlockStmt *actionBlk = dynamic_cast<BlockStmt *>(this->statement());
    if(actionBlk)
        f->nl();
    else
        f->space();

    this->statement()->prettyPrint(f);
    if(!actionBlk)
        f->space();
    f->printKw(L"تابع");
    if(actionBlk)
        f->nl(); // for extra neatness, add an empty line after block statements
}

void ReturnStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"ارجع ب:");
    f->space();
    this->returnVal()->prettyPrint(f);
}

void DelegationStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"وكل إلى");
    f->space();
    this->invokation()->prettyPrint(f);
}

void LaunchStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"شغل");
    f->space();
    this->invokation()->prettyPrint(f);
}


void LabelStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"علامة");
    f->space();
    this->target()->prettyPrint(f);
}

void GotoStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"اذهب إلى");
    f->space();
    if(!this->targetIsNumber)
        this->idTarget()->prettyPrint(f);
    else
        this->numericTarget()->prettyPrint(f);

}

void PrintStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"اطبع");
    if(this->fileObject() != NULL)
    {
        f->printKw(L"في");
        this->fileObject()->prettyPrint(f);
        f->colon();
        f->space();
    }
    commaSep(mapPrint(this->_args, this->_widths)).run(f);
    if(this->printOnSameLine)
    {
        f->comma();
        f->print("...");
    }
}

void ReadStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"اقرأ");

    if(this->fileObject() != NULL)
    {
        f->printKw(L"من");
        this->fileObject()->prettyPrint(f);
        f->colon();
        f->space();
    }

    // todo: minor loss of information
    // if user has a command read "", x, y
    // since "empty prompt" and "no prompt" have the same AST representation
    if(this->prompt !="")
    {
        f->print(strLiteralRepr(prompt));
        f->comma();
    }

    commaSep(mapRead(this->_variables, this->readNumberFlags)).run(f);
}

void DrawPixelStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"ارسم.نقطة");
    parens(&commaSep(&ast(this->x()), &ast(this->y()))).run(f);
    if(this->color() !=NULL)
    {
        f->comma();
        color()->prettyPrint(f);
    }
}

void DrawLineStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"ارسم.خط");
    parens(&commaSep(&ast(this->x1()), &ast(this->y1()))).run(f);
    f->print("-");
    parens(&commaSep(&ast(this->x2()), &ast(this->y2()))).run(f);
    if(this->color() !=NULL)
    {
        f->comma();
        color()->prettyPrint(f);
    }
}

void DrawRectStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"ارسم.مستطيل");
    parens(&commaSep(&ast(this->x1()), &ast(this->y1()))).run(f);
    f->print("-");
    parens(&commaSep(&ast(this->x2()), &ast(this->y2()))).run(f);

    if(this->color())
    {
        f->comma();
        color()->prettyPrint(f);
        if(this->filled())
        {
            f->comma();
            filled()->prettyPrint(f);
        }
    }
    else if(this->filled())
    {
        f->comma();
        f->comma();
        filled()->prettyPrint(f);
    }
}

void DrawCircleStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"ارسم.دائرة");
    parens(&commaSep(&ast(this->cx()), &ast(this->cy()))).run(f);
    f->comma();
    radius()->prettyPrint(f);
    if(this->color())
    {
        f->comma();
        color()->prettyPrint(f);
        if(this->filled())
        {
            f->comma();
            filled()->prettyPrint(f);
        }
    }
    else if(this->filled())
    {
        f->comma();
        f->comma();
        filled()->prettyPrint(f);
    }
}

void DrawSpriteStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"ارسم.طيف");
    this->sprite()->prettyPrint(f);
    f->space();
    f->printKw(L"في");
    parens(&commaSep(&ast(this->x()), &ast(this->y()))).run(f);
    /*
    f->openParen();
    this->x()->prettyPrint(f);
    f->comma();
    this->y()->prettyPrint(f);
    this->closeParen();
    */
}

void ZoomStmt::prettyPrint(CodeFormatter *f)
{

}

QStdWString translateEventType(EventType t)
{
    switch(t)
    {
    case KalimatKeyDownEvent:
        return L"ضغط.مفتاح";
    case KalimatKeyUpEvent:
        return L"رفع.مفتاح";
    case KalimatKeyPressEvent:
        return L"ادخال.حرف";
    case KalimatMouseUpEvent:
        return L"رفع.زر.ماوس";
    case KalimatMouseDownEvent:
        return L"ضغط.زر.ماوس";
    case KalimatMouseMoveEvent:
        return L"تحريك.ماوس";
    case KalimatSpriteCollisionEvent:
        return L"تصادم";
    }
}

void EventStatement::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"عند حادثة");
    f->printKw(translateEventType(this->type));
    f->printKw(L"نفذ");
    this->handler()->prettyPrint(f);
}

void SendStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"ارسل");
    if(signal)
        f->printKw(L"إشارة");
    else
        value()->prettyPrint(f);
    f->printKw(L"إلى");
    this->channel()->prettyPrint(f);
}

void ReceiveStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"تسلم");
    if(signal)
        f->printKw(L"إشارة");
    else
        value()->prettyPrint(f);
    f->printKw(L"من");
    this->channel()->prettyPrint(f);
}

void SelectStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"تخير");
    f->nl();
    f->indent();
    for(int i=0; i<count(); i++)
    {
        ChannelCommunicationStmt *cond = condition(i);
        Statement *act = action(i);
        BlockStmt *actBlk = dynamic_cast<BlockStmt *>(act);

        if(i>0)
            f->printKw(L"أو");
        cond->prettyPrint(f);
        f->space();
        f->colon();
        if(actBlk)
            f->nl();
        act->prettyPrint(f);
    }
    f->deindent();
    f->print(L"تم");
}

void BlockStmt::prettyPrint(CodeFormatter *f)
{
    f->indent();
    for(int i=0; i<statementCount(); i++)
    {
        if(statement(i)->attachedComments.trimmed() != "")
        {
            f->print("--");
            f->print(statement(i)->attachedComments);
        }
        statement(i)->prettyPrint(f);
        f->nl();
    }
    f->deindent();
}

void InvokationStmt::prettyPrint(CodeFormatter *f)
{
    this->expression()->prettyPrint(f);
}

void VarPattern::prettyPrint(CodeFormatter *f)
{
    id()->prettyPrint(f);
}

void ArrayPattern::prettyPrint(CodeFormatter *f)
{
    //todo: ArrayPattern::prettyPrint() by not copying to new vector
    QVector<PrettyPrintable *> data;
    for(int i=0; i<elementCount(); i++)
        data.append(element(i));

    brackets(&commaSep(mapFmt(this->_elements))).run(f);
}

void ObjPattern::prettyPrint(CodeFormatter *f)
{
    this->classId()->prettyPrint(f);
    f->space();
    f->printKw(_ws(L"له"));
    QVector<PrettyPrintable *> data;
    //todo: optimize ObjPattern::prettyPrint() by not copying to new vector
    for(int i=0; i<this->fieldCount(); i++)
    {
        data.append(fieldName(i));
        data.append(fieldPattern(i));
    }
    commaSepPairs(mapFmt(data), "=").run(f);
}

void MapPattern::prettyPrint(CodeFormatter *f)
{
    QVector<PrettyPrintable *> data;
    //todo: optimize ObjPattern::prettyPrint() by not copying to new vector
    for(int i=0; i<this->pairCount(); i++)
    {
        data.append(key(i));
        data.append(value(i));
    }
    braces(&commaSepPairs(mapFmt(data), "=>")).run(f);
}

QString translateOperator(QString op)
{
    if(op == "add")
    {
        return "+";
    }
    else if(op == "sub")
    {
        return "-";
    }
    else if(op == "mul")
    {
        return _ws(L"×");
    }
    else if(op == "div")
    {
        return _ws(L"÷");
    }
    else if(op == "and")
    {
        return _ws(L"وأيضا");
    }
    else if(op == "or")
    {
        return _ws(L"أو");
    }
    else if(op == "not")
    {
        return _ws(L"ليس");
    }
    else if(op == "lt")
    {
        return "<";
    }
    else if(op == "gt")
    {
        return ">";
    }
    else if(op == "le")
    {
        return "<=";
    }
    else if(op == "ge")
    {
        return ">=";
    }
    else if(op == "eq")
    {
        return "=";
    }
    else if(op == "ne")
    {
        return "<>";
    }
    return "?";
}

void BinaryOperation::prettyPrint(CodeFormatter *f)
{
    this->operand1()->prettyPrint(f);
    f->space();
    f->print(translateOperator(this->_operator));
    f->space();
    this->operand2()->prettyPrint(f);
}

void IsaOperation::prettyPrint(CodeFormatter *f)
{
    this->expression()->prettyPrint(f);
    f->space();
    f->printKw(L"هو");
    this->type()->prettyPrint(f);
}

void MatchOperation::prettyPrint(CodeFormatter *f)
{
    this->expression()->prettyPrint(f);
    f->space();
    f->print(_ws(L"~"));
    f->space();
    this->pattern()->prettyPrint(f);
}

void UnaryOperation::prettyPrint(CodeFormatter *f)
{
    f->print(translateOperator(this->_operator));
    f->space();
    this->operand()->prettyPrint(f);
}

void Identifier::prettyPrint(CodeFormatter *f)
{
    f->print(name);
}

void TypeIdentifier::prettyPrint(CodeFormatter *f)
{
    f->print(name);
}

void PointerTypeExpression::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"مشير");
    f->openParen();
    pointeeType()->prettyPrint(f);
    f->closeParen();
}

void FunctionTypeExpression::prettyPrint(CodeFormatter *f)
{
    if(retType())
        f->printKw(L"دالة");
    else
        f->printKw(L"إجراء");
    f->openParen();
    for(int i=0; i<argTypeCount(); i++)
    {
        argType(i)->prettyPrint(f);
        if(i<argTypeCount()-1)
            f->comma();
    }
    f->closeParen();
}

void NumLiteral::prettyPrint(CodeFormatter *f)
{
    f->print(repr());
}

void StrLiteral::prettyPrint(CodeFormatter *f)
{
    f->print(repr());
}

void NullLiteral::prettyPrint(CodeFormatter *f)
{
    f->print(repr());
}

void BoolLiteral::prettyPrint(CodeFormatter *f)
{
    f->printKwExpression(repr());
}

void ArrayLiteral::prettyPrint(CodeFormatter *f)
{
    brackets(&commaSep(mapFmt(this->_data))).run(f);
}

void MapLiteral::prettyPrint(CodeFormatter *f)
{
    braces(&commaSepPairs(mapFmt(this->_data), "=>")).run(f);
}

void Invokation::prettyPrint(CodeFormatter *f)
{
    this->functor()->prettyPrint(f);
    parens(&commaSep(mapFmt(this->_arguments))).run(f);
}

void MethodInvokation::prettyPrint(CodeFormatter *f)
{
    this->receiver()->prettyPrint(f);
    f->colon();
    f->space();
    this->methodSelector()->prettyPrint(f);
    parens(&commaSep(mapFmt(this->_arguments))).run(f);
}

void Idafa::prettyPrint(CodeFormatter *f)
{
    spaceSep(&ast(modaf()), &ast(modaf_elaih())).run(f);
}

void ArrayIndex::prettyPrint(CodeFormatter *f)
{
    this->array()->prettyPrint(f);
    brackets(&ast(this->index())).run(f);
}

void MultiDimensionalArrayIndex::prettyPrint(CodeFormatter *f)
{
    this->array()->prettyPrint(f);
    brackets(&commaSep(mapFmt(this->_indexes))).run(f);
}

void ObjectCreation::prettyPrint(CodeFormatter *f)
{
    className()->prettyPrint(f);
    f->space();
    f->printKw(L"جديد");
}

void ProcedureDecl::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"إجراء");
    procName()->prettyPrint(f);

    parens(&commaSep(mapFmt(this->_formals))).run(f);

    f->colon();
    f->nl();
    this->body()->prettyPrint(f);
    f->printKw(L"نهاية");
    f->nl(); // for extra neatness, add an empty line after definitions
}

void FunctionDecl::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"دالة");
    procName()->prettyPrint(f);

    parens(&commaSep(mapFmt(this->_formals))).run(f);

    f->colon();
    f->nl();
    this->body()->prettyPrint(f);
    f->printKw(L"نهاية");
    f->nl(); // for extra neatness, add an empty line after definitions
}

void FFILibraryDecl::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"مكتبة");
    f->print(QString("\"%1\"").arg(libName));
    f->colon();
    f->nl();
    f->indent();
    for(int i=0; i<declCount(); i++)
        decl(i)->prettyPrint(f);
    f->deindent();
    f->printKw(L"نهاية");
    f->nl(); // for extra neatness, add an empty line after definitions
}

void FFIProceduralDecl::prettyPrint(CodeFormatter *f)
{
    if(this->isFunctionNotProc)
        f->printKw(L"دالة");
    else
        f->printKw("إجراء");

    f->print(this->procName);
    f->space();
    if(symbol != procName)
    {
        f->printKw(L"برمز");
        f->print(QString("\"%1\"").arg(symbol));
    }
    f->openParen();
    for(int i=0; i<paramTypeCount(); i++)
    {
        paramType(i)->prettyPrint(f);
        if(i < paramTypeCount() -1)
            f->comma();
    }
    f->closeParen();
    if(isFunctionNotProc)
    {
        returnType()->prettyPrint(f);
    }

    f->nl();
}

void FFIStructDecl::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"فصيلة");
    this->name()->prettyPrint(f);
    f->colon();
    f->nl();
    f->indent();

    int index = 0;
    for(int i=0; i<fieldBatches.count(); i++)
    {
        f->printKw(L"له");

        for(int j=0; j< fieldBatches[i]; j++)
        {
            fieldName(index)->prettyPrint(f);
            f->space();
            f->printKw(L"هو");
            fieldType(index)->prettyPrint(f);
            index++;
        }
        f->nl();
    }
    f->deindent();
    f->printKw(L"نهاية");
    f->nl(); // for extra neatness, add an empty line after definitions
}

void Has::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"له");
    commaSep(mapFmt(this->fields)).run(f);
}

void RespondsTo::prettyPrint(CodeFormatter *f)
{
    if(isFunctions)
    {
        f->printKw(L"يرد على");
    }
    else
    {
        f->printKw(L"يستجيب ل:");
    }
    commaSep(mapFmt(methods)).run(f);
}

void ConcreteResponseInfo::prettyPrint(CodeFormatter *f)
{
    this->name.data()->prettyPrint(f);
    f->space();
    parens(&commaSep(mapFmt(params))).run(f);
}

void ClassDecl::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"فصيلة");
    this->name()->prettyPrint(f);
    f->colon();
    f->nl();
    f->indent();
    if(ancestor())
    {
        f->printKw(L"مبني على");
        ancestor()->prettyPrint(f);
        f->nl();
    }
    for(int i=0; i<_internalDecls.count(); i++)
    {
        ClassInternalDecl *id = _internalDecls[i].data();
        id->prettyPrint(f);
        f->nl();
    }
    f->deindent();
    f->printKw(L"نهاية");
    f->nl(); // for extra neatness, add an empty line after definitions
}

void GlobalDecl::prettyPrint(CodeFormatter *f)
{
    f->print(varName);
    f->space();
    f->printKw(L"مشترك");
}

void MethodDecl::prettyPrint(CodeFormatter *f)
{
    if(this->isFunctionNotProcedure)
        f->printKw(L"رد");
    else
        f->printKw(L"استجابة");

    this->className()->prettyPrint(f);
    f->space();
    this->receiverName()->prettyPrint(f);
    f->space();

    if(this->isFunctionNotProcedure)
        f->printKw(L"على");
    else
        f->printKw(L"ل:");

    procName()->prettyPrint(f);

    parens(&commaSep(mapFmt(this->_formals, 1))).run(f);

    f->colon();
    f->nl();
    this->body()->prettyPrint(f);
    f->printKw(L"نهاية");
    f->nl(); // for extra neatness, add an empty line after definitions
}

QVector<FormatMaker *> mapPrint(QVector<QSharedPointer<Expression > > args, QVector<QSharedPointer<Expression > > widths)
{
    QVector<FormatMaker *> ret;
    for(int i=0; i<args.count(); i++)
        ret.append(new PrintFmt(args[i].data(), widths[i].data()));
    return ret;
}

QVector<FormatMaker *> mapRead(QVector<QSharedPointer<AssignableExpression> > variables, QVector<bool> readNumberFlags)
{
    QVector<FormatMaker *> ret;
    for(int i=0; i<variables.count(); i++)
        ret.append(new ReadFmt(variables[i].data(), readNumberFlags[i]));
    return ret;
}

InvokationStmt::~InvokationStmt()
{

}

ReceiveStmt::~ReceiveStmt() {}
SendStmt::~SendStmt() {}
ZoomStmt::~ZoomStmt() {}
DrawSpriteStmt::~DrawSpriteStmt() {}
DrawCircleStmt::~DrawCircleStmt() {}
DrawRectStmt::~DrawRectStmt() {}
DrawLineStmt::~DrawLineStmt() {}
DrawPixelStmt::~DrawPixelStmt() {}
LaunchStmt::~LaunchStmt() {}
DelegationStmt::~DelegationStmt() {}
ReturnStmt::~ReturnStmt() {}
ForAllStmt::~ForAllStmt() {}
WhileStmt::~WhileStmt() {}
IfStmt::~IfStmt() {}
AssignmentStmt::~AssignmentStmt() {}
MatchOperation::~MatchOperation() {}
