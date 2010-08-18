/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include <QObject>
#include <QString>
#include <QStringList>
#include <QSet>
#include <QMap>
#include <QVector>
#include <QLocale>
#include "value.h"


ValueClass *Value::ObjectType = new ValueClass("Object", NULL);
ValueClass *Value::IntType = new ValueClass("Integer", Value::ObjectType);
ValueClass *Value::DoubleType = new ValueClass("Double", Value::ObjectType);
ValueClass *Value::MethodType = new ValueClass("Method", Value::ObjectType);
ValueClass *Value::ExternalMethodType = new ValueClass("ExternalMethod", Value::ObjectType);
ValueClass *Value::ClassType = new ValueClass("Class", Value::ObjectType);
ValueClass *Value::ArrayType = new ValueClass("Array", Value::ObjectType);
ValueClass *Value::StringType = new ValueClass("String", Value::ObjectType);
ValueClass *Value::SpriteType = new ValueClass("Sprite", Value::ObjectType);
ValueClass *Value::FileType = NULL;
ValueClass *Value::RawFileType = new ValueClass("RawFile", Value::ObjectType);
ValueClass *Value::RefType = new ValueClass("Reference", Value::ObjectType);
ValueClass *Value::FieldRefType = new ValueClass("FieldReference", Value::ObjectType);
ValueClass *Value::ArrayRefType = new ValueClass("ArrayReference", Value::ObjectType);
ValueClass *Value::NullType = new ValueClass("Null", Value::ObjectType);

Value *Value::NullValue;
Value::Value()
{
    mark = 0;
}
Value::~Value()
{
    // In the cases of RefVal, ArrayVal and StringVal
    // we _always_ assume the Value owns the stored pointer.

    // In the case of RawVal, we assume the external code that passed the RawVal
    // to the 'Value' object owns the passed RawVal.
    switch(this->tag)
    {
    case Int:
    case Double:
    case RawVal:
        break;
    case ObjectVal:
        delete v.objVal;
        break;
    case StringVal:
        delete v.strVal;
        break;
    case ArrayVal:
        delete[] v.arrayVal->Elements;
        delete v.arrayVal;
        break;
    case MultiDimensionalArrayVal:
        delete v.multiDimensionalArrayVal;
        break;
    case RefVal:
        delete v.refVal;
        break;
    }
}

QString Object::toString()
{
    return QString("%1").arg((long)this);
}
bool Object::hasSlot(QString name)
{
    return _slots.contains(name);
}
Value *Object::getSlotValue(QString name)
{
    return _slots[name];
}
void Object::setSlotValue(QString name, Value *val)
{
    _slots[name] = val;
}

void FieldReference::Set(Value *v)
{
    object->setSlotValue(SymRef, v);
}
Value *FieldReference::Get()
{
    return object->getSlotValue(SymRef);
}

void ArrayReference::Set(Value *val)
{
    array->Elements[index] = val;
}
Value *ArrayReference::Get()
{
    return array->Elements[index];
}

void MultiDimensionalArrayReference::Set(Value *val)
{
    array->set(index, val);
}
Value *MultiDimensionalArrayReference::Get()
{
    return array->get(index);
}

int Value::unboxInt()
{
    return v.intVal;
}

double Value::unboxDouble()
{
    return v.doubleVal;
}
Object *Value::unboxObj()
{
    return v.objVal;
}
VArray *Value::unboxArray()
{
    return v.arrayVal;
}
MultiDimensionalArray<Value *> *Value::unboxMultiDimensionalArray()
{
    return v.multiDimensionalArrayVal;
}

QString *Value::unboxStr()
{
    return v.strVal;
}

void *Value::unboxRaw()
{
    return v.rawVal;
}

Reference *Value::unboxRef()
{
    return v.refVal;
}

QString ArrayToString(VArray *arr)
{
    QStringList lst;
    for(int i=0; i<arr->count; i++)
        lst.append(arr->Elements[i]->toString());
    return QString("[%1]").arg(lst.join(", "));
}

QString Value::toString()
{
    QString *sv = NULL;
    QString ret = "<unprintable value>";
    void *val ;
    Value *v = this;
    QLocale loc(QLocale::Arabic, QLocale::Egypt);
    switch(v->tag)
    {
    case Int:
        ret = loc.toString(v->unboxInt());
        break;
    case Double:
        ret = QString("%1").arg(v->unboxDouble());
        break;
    case ObjectVal:
        ret = QString("<%1>").arg(v->type->getName());
        break;
    case StringVal:
        val = v->unboxStr();
        sv = (QString *) val;
        ret = *sv;
        break;
    case NullVal:
        ret = QString::fromWCharArray(L"<لاشيء>");
        break;
    case RawVal:
        ret = QString("<raw %1>").arg((long) v->unboxRaw());
        break;
    case RefVal:
        ret = "<a reference>";
    case ArrayVal:
        ret = ArrayToString(v->unboxArray());
        break;
    }
    QString str = ret;
    return str;
}

ValueClass::ValueClass(QString name, ValueClass *baseClass)
{
    this->name = name;
    if(baseClass !=NULL)
        this->BaseClasses.append(baseClass);

}
bool ValueClass::hasSlot(QString name)
{
    return false;
}
Value *ValueClass::getSlotValue(QString name)
{
    return NULL;
}
void ValueClass::setSlotValue(QString name, Value *val)
{

}

QString ValueClass::getName()
{
    return name;
}
bool ValueClass::hasField(QString name)
{
    return fields.contains(name);
}
IClass *ValueClass::baseClass()
{
    if(BaseClasses.count() == 0)
        return NULL;
    return BaseClasses[0];
}
bool ValueClass::subclassOf(ValueClass *c)
{
    if(c == this)
        return true;

    for(int i=0; i<BaseClasses.count(); i++)
    {
        if(BaseClasses[i]->subclassOf(c))
            return true;
    }
    return  false;
}

Method *ValueClass::lookupMethod(QString name)
{
    if(methods.contains(name))
        return (Method *) methods[name]->v.objVal;
    for(int i=0; i<BaseClasses.count(); i++)
    {
        Method *ret = BaseClasses[i]->lookupMethod(name);
        if(ret != NULL)
            return ret;
    }
    return NULL;
}

ValueClass::~ValueClass()
{

}
QString ValueClass::toString()
{
    return "class "+ name;
}

