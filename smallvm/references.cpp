#include "references.h"

FieldReference::FieldReference(IObject *_object, QString fieldName)
{
    object = _object; SymRef = fieldName;
}

void FieldReference::Set(Value *v)
{
    object->setSlotValue(SymRef, v);
}

Value *FieldReference::Get()
{
    return object->getSlotValue(SymRef);
}

ArrayReference::ArrayReference(VArray *arr, int ind)
{
    array = arr; index = ind;
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

VariableRef::VariableRef(Frame *frame, QString varName, int fastLocalIndex)
    :frame(frame), varName(varName), fastLocalIndex(fastLocalIndex)
{

}

void VariableRef::Set(Value *val)
{
    frame->fastLocals[fastLocalIndex] = val;
}

Value *VariableRef::Get()
{
    return frame->fastLocals[fastLocalIndex];
}
