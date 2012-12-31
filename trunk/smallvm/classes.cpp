#include "classes.h"
#include "allocator.h"
#include "vmerror.h"
#include "runtime_identifiers.h"

IObject *ValueClass::newValue(Allocator *allocator)
{
    Object *newObj = new Object();
    InitObjectLayout(newObj, allocator);
    return newObj;
}

bool ValueClass::getFieldAttribute(QString attr, Value *&ret, Allocator *allocator)
{
    if(!fieldAttributes.contains(attr))
        return false;
    ret = fieldAttributes[attr];
    return true;
}

QVector<PropertyDesc> ValueClass::getProperties()
{
    return properties;
}

void ValueClass::InitObjectLayout(Object *object, Allocator *allocator)
{
    InitObjectLayoutHelper(this, object, allocator);
}

void ValueClass::InitObjectLayoutHelper(ValueClass *_class, Object *object, Allocator *allocator)
{
    //todo:
    for(QVector<IClass *>::iterator i=_class->BaseClasses.begin(); i!=_class->BaseClasses.end(); ++i)
    {
        IClass *aBase = *i;
        ValueClass *vc = dynamic_cast<ValueClass *>(aBase);
        if(vc)
            vc->InitObjectLayout(object, allocator);
    }
    for(QSet<QString>::iterator i = _class->fields.begin(); i!=_class->fields.end();  ++i)
    {
        object->setSlotValue(*i, allocator->null());
    }
    for(QVector<QString>::iterator i = _class->fieldNames.begin(); i!=_class->fieldNames.end(); ++i)
    {
        object->slotNames.append(*i);
    }

}

PointerClass::PointerClass(IClass *pointee)
{
    this->pointee = pointee;
}

bool PointerClass::hasSlot(QString name)
{
    return false;
}

QList<QString> PointerClass::getSlotNames()
{
    return QList<QString>();
}

Value *PointerClass::getSlotValue(QString name)
{
    return NULL;
}

void PointerClass::setSlotValue(QString name, Value *val)
{

}

QString PointerClass::getName()
{
    return VMId::get(RId::PointerOf1, pointee->getName());
}

bool PointerClass::hasField(QString name)
{
    return false;
}

IClass *PointerClass::baseClass()
{
    return BuiltInTypes::ObjectType;
}

bool PointerClass::subclassOf(IClass *c)
{
    // We assume "Pointer" is covariant
    // todo: revise this decision
    if(c == BuiltInTypes::ObjectType)
        return true;
    PointerClass *c2 = dynamic_cast<PointerClass *>(c);
    if(!c2)
        return false;
    return pointee->subclassOf(c2->pointee);
}

IMethod *PointerClass::lookupMethod(QString name)
{
    return baseClass()->lookupMethod(name);
}

IObject *PointerClass::newValue(Allocator *allocator)
{
    throw VMError(InternalError);
}

bool PointerClass::getFieldAttribute(QString attr, Value *&ret, Allocator *allocator)
{
    return false;
}

QVector<PropertyDesc> PointerClass::getProperties()
{
    return QVector<PropertyDesc>();
}

QString PointerClass::toString()
{
    return getName();
}

FunctionClass::FunctionClass(IClass *retType, QVector<IClass *> argTypes)
    :retType(retType),
      argTypes(argTypes)
{
}

bool FunctionClass::hasSlot(QString name)
{
    return false;
}

QList<QString> FunctionClass::getSlotNames()
{
    return QList<QString>();
}

Value *FunctionClass::getSlotValue(QString name)
{
    return NULL;
}

void FunctionClass::setSlotValue(QString name, Value *val)
{

}

QString FunctionClass::getName()
{
    QString kind = retType? _ws(L"دالة"): _ws(L"إجراء");
    QStringList args;
    for(int i=0; i<argTypes.count(); ++i)
    {
        args.append(argTypes[i]->getName());
    }

    return QString("%1(%2) %3")
            .arg(kind)
            .arg(args.join(", "))
            .arg(retType? retType->getName(): "");
}

bool FunctionClass::hasField(QString name)
{
    return false;
}

IClass *FunctionClass::baseClass()
{
    return BuiltInTypes::ObjectType;
}

bool FunctionClass::subclassOf(IClass *c)
{
    // We assume "Function" is invariant
    // todo: revise this decision
    if(c == BuiltInTypes::ObjectType)
        return true;
    FunctionClass *f2 = dynamic_cast<FunctionClass*>(c);
    if(!f2)
        return false;

    return f2 == this;
}

IMethod *FunctionClass::lookupMethod(QString name)
{
    return baseClass()->lookupMethod(name);
}

IObject *FunctionClass::newValue(Allocator *allocator)
{
    throw VMError(InternalError);
}

bool FunctionClass::getFieldAttribute(QString attr, Value *&ret, Allocator *allocator)
{
    return false;
}

QVector<PropertyDesc> FunctionClass::getProperties()
{
    return QVector<PropertyDesc>();
}

QString FunctionClass::toString()
{
    return getName();
}
