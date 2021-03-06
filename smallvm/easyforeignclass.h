#ifndef EASYFOREIGNCLASS_H
#define EASYFOREIGNCLASS_H

#ifndef CLASSES_H
    #include "classes.h"
#endif

#include <QMetaClassInfo>

class EasyForeignClass;
class Process;
#define _method(a, b, c) methodIds[a] = b; \
    methodArities[a] = c + 1;

class ForeignMethodProxy : public IForeignMethod
{
    EasyForeignClass *owner;
    int arity;
    int id;
    QString name;
public:
    ForeignMethodProxy(QString name, EasyForeignClass *owner, int id, int arity);
    QString toString();
    Value *invoke(Process *proc, QVector<Value *> args);
    int Arity();
};

class VM;
class EasyForeignClass : public ForeignClass
{
protected:
    QVector<PropertyDesc> properties;
    QSet<QString> fields;
    QMap<QString, int> methodIds;
    QMap<QString, int> methodArities;
    QMap<QString, IMethod *> vmMethods;
    VM *vm;
public:
    EasyForeignClass(QString className, VM *vm);
    EasyForeignClass(QString className);

    void attachVmMethod(QString methodName);
    void attachVmMethod(QString className, QString methodName);
    void attachVmMethod(VM *vm, QString className, QString methodName);
    void attachVmMethod(VM *vm, QString methodName);
    virtual Value *dispatch(Process *proc, int id, QVector<Value *>args) = 0;
    virtual IObject *newValue(Allocator *allocator) = 0;
    virtual bool getFieldAttribute(QString attr, Value *&ret, Allocator *allocator) {return false;}
    virtual QVector<PropertyDesc> getProperties() { return properties; }
    //IClass
    bool hasField(QString name);
    IMethod *lookupMethod(QString name);
};

#endif // EASYFOREIGNCLASS_H
