/**************************************************************************
**   The Kalimat programming language
**   Copyright 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/


#include "runwindow.h"
#include "builtinmethods.h"
#include "guieditwidgethandler.h"

#include <QApplication>
#include <QDialog>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QLabel>
#include <QFrame>
#include <QScrollArea>
#include <QGroupBox>
#include <QVBoxLayout>

#include <QString>
#include <QMap>
#include <QPainter>
#include <QFile>
#include <QTextStream>

#include <QLibrary>
#include <math.h>
#include <algorithm>
#include <QPushButton>
#include <QVariant>
#include <QtConcurrentRun>
#include <QMessageBox>
#include <iostream>
#include "../smallvm/vm_ffi.h"
using namespace std;

#define _ws(s) QString::fromStdWString(s)

void PrintProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    if(stack.empty())
        vm->signal(InternalError1, "Empty operand stack when reading value to in 'print'");
    Value *v = stack.pop();
    QString str = v->toString();
    w->textLayer.print(str);
}

void PushReadChanProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    stack.push(w->readChannel);
}

void MouseEventChanProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    stack.push(w->mouseEventChannel);
}

void MouseDownEventChanProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    stack.push(w->mouseDownEventChannel);
}

void MouseUpEventChanProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    stack.push(w->mouseUpEventChannel);
}


void KbEventChanProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    stack.push(w->kbEventChannel);
}

WindowReadMethod::WindowReadMethod(RunWindow *parent, VM *vm)
{
    this->parent = parent;
    this->vm = vm;
    this->readNum = false;
}

void WindowReadMethod::operator ()(QStack<Value *> &operandStack)
{
    readNum = popInt(operandStack, parent, vm);
    parent->beginInput();
    parent->update(); // We must do this, because normal updating is done
                      // by calling redrawWindow() in the instruction loop, and
                      // here we suspend the instruction loop...
}

WindowProxyMethod::WindowProxyMethod(RunWindow *parent, VM *vm, VM_PROC proc)
{
    this->vm = vm;
    this->parent = parent;
    this->proc = proc;
}

void WindowProxyMethod::operator ()(QStack<Value *> &operandStack)
{
    proc(operandStack, parent, vm);
}

void SetCursorPosProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    int line = popInt(stack, w, vm);
    int col = popInt(stack, w, vm);
    bool result = w->textLayer.setCursorPos(line, col);
    w->assert(result, ArgumentError, QString::fromStdWString(L"قيم غير صحيحة لتحديد موقع المؤشر"));
}
void GetCursorRowProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    int r = w->textLayer.getCursorRow();
    stack.push(vm->GetAllocator().newInt(r));
}
void GetCursorColProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    int c = w->textLayer.getCursorCol();
    stack.push(vm->GetAllocator().newInt(c));
}


void PrintUsingWidthProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    Value *v = stack.pop();
    w->typeCheck(stack.top(), BuiltInTypes::IntType);
    int wid = stack.pop()->unboxInt();
    QString str = v->toString();
    w->textLayer.print(str, wid);
}

void DrawPixelProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    int x = popIntOrCoercable(stack, w, vm);
    int y = popIntOrCoercable(stack, w, vm);
    int color = popInt(stack, w, vm);
    if(color == -1)
        color = 0;
    QColor clr = w->paintSurface->GetColor(color);
    w->paintSurface->TX(x);
    QPainter p(w->paintSurface->GetImage());
    p.fillRect(x, y, 1, 1, clr);
    w->redrawWindow();
}

void DrawLineProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    int x1 = popIntOrCoercable(stack, w, vm);
    int y1 = popIntOrCoercable(stack, w, vm);
    int x2 = popIntOrCoercable(stack, w, vm);
    int y2 = popIntOrCoercable(stack, w, vm);

    w->paintSurface->TX(x1);
    w->paintSurface->TX(x2);
    int color = popInt(stack, w, vm);
    if(color ==-1)
        color = 0;

    QColor clr = w->paintSurface->GetColor(color);
    QPainter p(w->paintSurface->GetImage());

    QColor oldcolor = p.pen().color();
    QPen pen = p.pen();
    pen.setColor(clr);
    p.setPen(pen);
    p.drawLine(x1, y1, x2, y2);
    pen.setColor(oldcolor);
    p.setPen(pen);

    w->redrawWindow();
}

void DrawRectProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    int x1 = popIntOrCoercable(stack, w, vm);
    int y1 = popIntOrCoercable(stack, w, vm);
    int x2 = popIntOrCoercable(stack, w, vm);
    int y2 = popIntOrCoercable(stack, w, vm);

    w->paintSurface->TX(x1);
    w->paintSurface->TX(x2);
    int color = popInt(stack, w, vm);
    bool filled = popBool(stack, w, vm);

    if(color ==-1)
        color = 0;

    QPainter p(w->paintSurface->GetImage());

    int top = min(y1, y2);
    int left = min(x1, x2);
    int wid = abs(x2-x1);

    int hei = abs(y2-y1);
    QColor clr = w->paintSurface->GetColor(color);
    QColor oldcolor = p.pen().color();
    QPen pen = p.pen();


    pen.setColor(clr);
    p.setPen(pen);

    if(filled)
    {
        QBrush oldBrush = p.brush();
        p.setBrush(QBrush(clr,Qt::SolidPattern));
        p.drawRect(left, top, wid, hei);
        p.setBrush(oldBrush);
    }
    else
    {
        p.setBrush(Qt::NoBrush);
        p.drawRect(left, top, wid, hei);
    }

    pen.setColor(oldcolor);
    p.setPen(pen);


    w->redrawWindow();
}

void DrawCircleProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    int cx = popIntOrCoercable(stack, w, vm);
    int cy = popIntOrCoercable(stack, w, vm);

    int radius = popIntOrCoercable(stack, w, vm);

    int color = popInt(stack, w, vm);

    bool filled = popBool(stack, w, vm);
    w->paintSurface->TX(cx);
    QPainter p(w->paintSurface->GetImage());
    if(color ==-1)
        color = 0;
    QColor clr = w->paintSurface->GetColor(color);
    QColor oldcolor = p.pen().color();
    QPen pen = p.pen();


    pen.setColor(clr);
    p.setPen(pen);

    if(filled)
    {
        QBrush oldBrush = p.brush();
        p.setBrush(QBrush(clr,Qt::SolidPattern));
        p.drawEllipse(cx-radius, cy-radius, radius*2, radius*2);
        p.setBrush(oldBrush);
    }
    else
    {
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(cx-radius, cy-radius, radius*2, radius*2);
    }
    pen.setColor(oldcolor);
    p.setPen(pen);


    w->redrawWindow();
}

void RandomProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    int max = popIntOrCoercable(stack, w, vm);
    vm->assert(max >0, ArgumentError, QString::fromStdWString(L"دالة عشوائي لابد أن تأخذ قيمة موجبة"));
    int ret = rand()%max;
    stack.push(vm->GetAllocator().newInt(ret));
}

Value *ConvertStringToNumber(QString str, VM *vm)
{
    bool ok;
    QLocale loc(QLocale::Arabic, QLocale::Egypt);
    int i = loc.toInt(str, &ok,10);
    if(ok)
    {
       return vm->GetAllocator().newInt(i);
    }

    long lng = loc.toLongLong(str, &ok, 10);
    if(ok)
    {
        return vm->GetAllocator().newLong(lng);
    }

    i = str.toInt(&ok, 10);

    if(ok)
    {
        return vm->GetAllocator().newInt(i);
    }
    double d = str.toDouble(&ok);
    if(ok)
    {
        return vm->GetAllocator().newDouble(d);
    }
    return NULL;
}

void ToNumProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{

    QString *str = popString(stack, w, vm);
    Value * v = ConvertStringToNumber(*str, vm);

    if(v != NULL)
        stack.push(v);
    else
    {
        vm->signal(ArgumentError, QString::fromStdWString(L"لا يمكن تحويل النص \"%1\" إلى قيمة عددية").arg(*str));
    }

}

void ConcatProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    QString *str1 = popString(stack, w, vm);
    QString *str2 = popString(stack, w, vm);


    QString *ret = new QString((*str1)+(*str2));
    stack.push(vm->GetAllocator().newString(ret));

}

void StrLenProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    QString *str = popString(stack, w, vm);

    int ret = str->length();
    stack.push(vm->GetAllocator().newInt(ret));
}

void StrFirstProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    int n = popInt(stack, w, vm);
    QString *str = popString(stack, w, vm);

    QString *ret = new QString(str->left(n));
    stack.push(vm->GetAllocator().newString(ret));
}

void StrLastProc(QStack<Value *> &stack, RunWindow *w,VM *vm)
{
    int n = popInt(stack, w, vm);

    QString *str = popString(stack, w, vm);

    QString *ret = new QString(str->right(n));
    stack.push(vm->GetAllocator().newString(ret));
}

void StrMidProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    QString *str = popString(stack, w, vm);


    int i = popInt(stack, w, vm);

    int n = popInt(stack, w, vm);

    // We make indexing one-based instead of QT's zero-based
    // todo: range checking in StrMidProc()
    QString *ret = new QString(str->mid(i -1 ,n));
    stack.push(vm->GetAllocator().newString(ret));
}

void StrBeginsWithProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    QString *strMain = popString(stack, w, vm);
    QString *strSub = popString(stack, w, vm);

    bool ret = strMain->startsWith(*strSub, Qt::CaseSensitive);
    stack.push(vm->GetAllocator().newBool(ret));
}

void StrEndsWithProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    QString *strMain = popString(stack, w, vm);
    QString *strSub = popString(stack, w, vm);

    bool ret = strMain->endsWith(*strSub, Qt::CaseSensitive);
    stack.push(vm->GetAllocator().newBool(ret));
}

void StrContainsProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    QString *strMain = popString(stack, w, vm);
    QString *strSub = popString(stack, w, vm);

    bool ret = strMain->contains(*strSub, Qt::CaseSensitive);
    stack.push(vm->GetAllocator().newBool(ret));
}

void StrSplitProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    QString *str = popString(stack, w, vm);
    QString *separator = popString(stack, w, vm);

    QStringList result = str->split(*separator, QString::KeepEmptyParts);
    Value *ret = vm->GetAllocator().newArray(result.count());
    for(int i=0; i<result.count(); i++)
    {
        ret->v.arrayVal->Elements[i] = vm->GetAllocator().newString(new QString(result[i]));
    }
    stack.push(ret);
}

void StrTrimProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    QString *str = popString(stack, w, vm);
    QString *str2 = new QString(str->trimmed());
    Value *ret = vm->GetAllocator().newString(str2);
    stack.push(ret);
}

void ToStringProc(QStack<Value *> &stack, RunWindow *, VM *vm)
{
    verifyStackNotEmpty(stack, vm);
    Value *v = stack.pop();
    QString ret;
    switch(v->tag)
    {
    case Int:
        ret = QString("%1").arg(v->unboxInt());
        break;
    case Long:
        ret = QString("%1").arg(v->unboxLong());
        break;
    case Double:
        ret = QString("%1").arg(v->unboxDouble());
        break;
    case StringVal:
        ret = QString(*v->unboxStr());
        break;
    case RawVal:
        ret = QString("%1").arg((long)v->unboxRaw());
        break;
    case ObjectVal:
        ret = QString("%1").arg(v->unboxObj()->toString());
        break;
    default:
        break;
    }
    stack.push(vm->GetAllocator().newString(new QString(ret)));
}

void RoundProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    double d = popDoubleOrCoercable(stack, w, vm);
    int i = (int) d;
    stack.push(vm->GetAllocator().newInt(i));

}

void RemainderProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    int n1 = popInt(stack, w, vm);
    int n2 = popInt(stack, w, vm);

    if(n2 == 0)
        vm->signal(DivisionByZero);
    int i = n1 % n2;
    stack.push(vm->GetAllocator().newInt(i));
}

int popIntOrCoercable(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    if(stack.empty())
    {
        vm->signal(InternalError1, "Empty operand stack when reading value");
    }
    Value *v = stack.pop();
    if(v->tag != Int && v->tag != Double && v->tag != Long)
    {
        w->typeError(BuiltInTypes::NumericType, v->type);
    }
    if(v->tag == Double)
        v = vm->GetAllocator().newInt((int) v->unboxDouble());
    if(v->tag == Long)
        v = vm->GetAllocator().newInt((int) v->unboxLong());

    return v->unboxInt();
}

double popDoubleOrCoercable(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    if(stack.empty())
    {
        vm->signal(InternalError1, "Empty operand stack when reading double or double-coercible value");
    }
    Value *v = stack.pop();
    if(v->tag != Int && v->tag != Double && v->tag != Long)
    {
        w->typeError(BuiltInTypes::NumericType, v->type);
    }
    if(v->tag == Int)
        v = vm->GetAllocator().newDouble(v->unboxInt());
    if(v->tag == Long)
        v = vm->GetAllocator().newDouble(v->unboxLong());
    return v->unboxDouble();
}

void verifyStackNotEmpty(QStack<Value *> &stack, VM *vm)
{
    if(stack.empty())
    {
        vm->signal(InternalError1, "Empty operand stack");
    }
}

void SinProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{

    double theta = popDoubleOrCoercable(stack, w, vm);

    double result = sin(theta);
    stack.push(vm->GetAllocator().newDouble(result));

}
void CosProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    double theta = popDoubleOrCoercable(stack, w, vm);

    double result = cos(theta);
    stack.push(vm->GetAllocator().newDouble(result));

}
void TanProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    double theta = popDoubleOrCoercable(stack, w, vm);

    double result = tan(theta);
    stack.push(vm->GetAllocator().newDouble(result));

}
void ASinProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{

    double theta = popDoubleOrCoercable(stack, w, vm);

    double result = asin(theta);
    stack.push(vm->GetAllocator().newDouble(result));

}
void ACosProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    double theta = popDoubleOrCoercable(stack, w, vm);

    double result = acos(theta);
    stack.push(vm->GetAllocator().newDouble(result));

}
void ATanProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    double theta = popDoubleOrCoercable(stack, w, vm);

    double result = atan(theta);
    stack.push(vm->GetAllocator().newDouble(result));

}
void SqrtProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    double theta = popDoubleOrCoercable(stack, w, vm);
    double result = sqrt(theta);
    stack.push(vm->GetAllocator().newDouble(result));

}
void Log10Proc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    double theta = popDoubleOrCoercable(stack, w, vm);
    double result = log10(theta);
    stack.push(vm->GetAllocator().newDouble(result));

}
void LnProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    double theta = popDoubleOrCoercable(stack, w, vm);
    double result = log(theta);
    stack.push(vm->GetAllocator().newDouble(result));

}

void LoadImageProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    verifyStackNotEmpty(stack, vm);
    w->typeCheck(stack.top(), BuiltInTypes::StringType);
    QString *fname = stack.pop()->unboxStr();
    *fname = w->ensureCompletePath(*fname);

    if(!QFile::exists(*fname))
    {
        w->assert(false, ArgumentError, QString::fromStdWString(L"تحميل صورة من ملف غير موجود"));
    }
    IClass *imgClass = dynamic_cast<IClass *>(vm->GetType(_ws(L"صورة"))->unboxObj());
    QImage *img = new QImage(*fname);
    IObject *obj = imgClass->newValue(&vm->GetAllocator());
    obj->setSlotValue("handle", vm->GetAllocator().newRaw(img, BuiltInTypes::ObjectType));
    stack.push(vm->GetAllocator().newObject(obj, imgClass));
}

void LoadSpriteProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    verifyStackNotEmpty(stack, vm);
    w->typeCheck(stack.top(), BuiltInTypes::StringType);
    QString *fname = stack.pop()->unboxStr();
    *fname = w->ensureCompletePath(*fname);

    if(!QFile::exists(*fname))
    {
        w->assert(false, ArgumentError, QString::fromStdWString(L"تحميل طيف من ملف غير موجود '%1'")
                  .arg(*fname));
    }
    Sprite *sprite = new Sprite(*fname);
    w->spriteLayer.AddSprite(sprite);
    stack.push(vm->GetAllocator().newRaw(sprite, BuiltInTypes::SpriteType));
}

void DrawImageProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    IClass *imgClass = dynamic_cast<IClass *>(vm->GetType(_ws(L"صورة"))->unboxObj());

    w->typeCheck(stack.top(), imgClass);
    IObject *obj = stack.pop()->unboxObj();
    QImage *handle = reinterpret_cast<QImage*>
            (obj->getSlotValue("handle")->unboxRaw());

    int x = popIntOrCoercable(stack, w , vm);
    int y = popIntOrCoercable(stack, w , vm);

    w->paintSurface->TX(x);
    x-= handle->width();

    QPainter p(w->paintSurface->GetImage());

    p.drawImage(x, y, *handle);

    w->redrawWindow();

}

void DrawSpriteProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    w->typeCheck(stack.top(), BuiltInTypes::SpriteType);
    void *rawVal = stack.pop()->unboxRaw();
    Sprite  *sprite = (Sprite *) rawVal;

    int x = popIntOrCoercable(stack, w , vm);
    int y = popIntOrCoercable(stack, w , vm);

    w->paintSurface->TX(x);
    x-= sprite->image.width();
    sprite->location = QPoint(x,y);
    sprite->visible = true;
    w->checkCollision(sprite);
    w->spriteLayer.showSprite(sprite);
    w->redrawWindow();
}

void ShowSpriteProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    w->typeCheck(stack.top(), BuiltInTypes::SpriteType);
    void *rawVal = stack.pop()->unboxRaw();
    Sprite  *sprite = (Sprite *) rawVal;

    sprite->visible = true;
    w->spriteLayer.showSprite(sprite);
    w->checkCollision(sprite);
    w->redrawWindow();
}

void HideSpriteProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    w->typeCheck(stack.top(), BuiltInTypes::SpriteType);
    void *rawVal = stack.pop()->unboxRaw();
    Sprite  *sprite = (Sprite *) rawVal;

    sprite->visible = false;
    w->spriteLayer.hideSprite(sprite);
    w->redrawWindow();
}
void GetSpriteLeftProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    w->typeCheck(stack.top(), BuiltInTypes::SpriteType);
    void *rawVal = stack.pop()->unboxRaw();
    Sprite  *sprite = (Sprite *) rawVal;

    int ret = sprite->boundingRect().left();
    w->paintSurface->TX(ret);
    stack.push(vm->GetAllocator().newInt(ret));
}

void GetSpriteRightProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    w->typeCheck(stack.top(), BuiltInTypes::SpriteType);
    void *rawVal = stack.pop()->unboxRaw();
    Sprite  *sprite = (Sprite *) rawVal;

    int ret = sprite->boundingRect().right();
    w->paintSurface->TX(ret);
    stack.push(vm->GetAllocator().newInt(ret));
}

void GetSpriteTopProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    w->typeCheck(stack.top(), BuiltInTypes::SpriteType);
    void *rawVal = stack.pop()->unboxRaw();
    Sprite  *sprite = (Sprite *) rawVal;

    int ret = sprite->boundingRect().top();
    stack.push(vm->GetAllocator().newInt(ret));
}

void GetSpriteBottomProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    w->typeCheck(stack.top(), BuiltInTypes::SpriteType);
    void *rawVal = stack.pop()->unboxRaw();
    Sprite  *sprite = (Sprite *) rawVal;

    int ret = sprite->boundingRect().bottom();
    stack.push(vm->GetAllocator().newInt(ret));
}

void GetSpriteWidthProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    w->typeCheck(stack.top(), BuiltInTypes::SpriteType);
    void *rawVal = stack.pop()->unboxRaw();
    Sprite  *sprite = (Sprite *) rawVal;

    int ret = sprite->boundingRect().width();
    stack.push(vm->GetAllocator().newInt(ret));
}

void GetSpriteHeightProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    w->typeCheck(stack.top(), BuiltInTypes::SpriteType);
    void *rawVal = stack.pop()->unboxRaw();
    Sprite  *sprite = (Sprite *) rawVal;

    int ret = sprite->boundingRect().height();
    stack.push(vm->GetAllocator().newInt(ret));
}

void GetSpriteImageProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    w->typeCheck(stack.top(), BuiltInTypes::SpriteType);
    void *rawVal = stack.pop()->unboxRaw();
    Sprite  *sprite = (Sprite *) rawVal;

    QString clsName = QString::fromStdWString(L"صورة");
    QImage *img = new QImage(sprite->image.toImage());
    IClass *imgClass = dynamic_cast<IClass *>
            (vm->GetType(clsName)->unboxObj());
    IObject *imgObj = imgClass->newValue(&vm->GetAllocator());
    imgObj->setSlotValue("handle", vm->GetAllocator().newRaw(img, BuiltInTypes::ObjectType));

    stack.push(vm->GetAllocator().newObject(imgObj, imgClass));
}

void SetSpriteImageProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    QString clsName = QString::fromStdWString(L"صورة");
    IClass *imgClass = dynamic_cast<IClass *>
            (vm->GetType(clsName)->unboxObj());

    w->typeCheck(stack.top(), BuiltInTypes::SpriteType);
    void *rawVal = stack.pop()->unboxRaw();
    Sprite  *sprite = (Sprite *) rawVal;

    w->typeCheck(stack.top(), imgClass);
    IObject *imgObj = stack.pop()->unboxObj();
    QImage *img = reinterpret_cast<QImage *>
            (imgObj->getSlotValue("handle")->unboxRaw());

    sprite->setImage(QPixmap::fromImage(*img));
    w->redrawWindow();
}

void WaitProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    /*
    int ms = stack.pop()->unboxNumeric();
    // The GC could collect the channel
    // even when the RunWindow is still to send it a message
    // therefore in the wait() builtin
    // we shall store a reference to it
    // in a local variable

    Value *channel = vm->GetAllocator().newChannel(false);
    //w->suspend();
    int cookie = w->startTimer(ms);
    w->setAsleep(cookie, channel, ms);
    stack.push(channel);
    */
    int ms = stack.pop()->unboxNumeric();
    Process *proc = vm->currentProcess();
    vm->makeItSleep(proc, ms);
}

void ZoomProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    int x1 = popIntOrCoercable(stack, w , vm);
    int y1 = popIntOrCoercable(stack, w , vm);
    int x2 = popIntOrCoercable(stack, w , vm);
    int y2 = popIntOrCoercable(stack, w , vm);
    //w->TX(x1);
    //w->TX(x2);
    w->paintSurface->zoom(x1, y1, x2, y2);
}

void ClsProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    w->cls();
}

void ClearTextProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    w->clearAllText();
}

void SetTextColorProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    int color = popInt(stack, w, vm);
    w->assert(color>=0 && color <=15, ArgumentError, "Color value must be from 0 to 15");
    w->paintSurface->setTextColor(w->paintSurface->GetColor(color));
    w->redrawWindow();
}

void BuiltInConstantProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    QString *constName = popString(stack, w, vm);
    if(constName->operator ==(QString::fromStdWString(L"سطر.جديد")))
    {
        stack.push(vm->GetAllocator().newString(new QString("\n")));
        return;
    }
    if((*constName) ==(QString::fromStdWString(L"c_int")))
    {
        stack.push(vm->GetAllocator().newObject(BuiltInTypes::c_int, BuiltInTypes::ClassType, false));
        return;
    }
    if((*constName) ==(QString::fromStdWString(L"c_long")))
    {
        stack.push(vm->GetAllocator().newObject(BuiltInTypes::c_long, BuiltInTypes::ClassType, false));
        return;
    }
    if((*constName) ==(QString::fromStdWString(L"c_float")))
    {
        stack.push(vm->GetAllocator().newObject(BuiltInTypes::c_float, BuiltInTypes::ClassType, false));
        return;
    }
    if((*constName) ==(QString::fromStdWString(L"c_double")))
    {
        stack.push(vm->GetAllocator().newObject(BuiltInTypes::c_double, BuiltInTypes::ClassType, false));
        return;
    }
    if((*constName) ==(QString::fromStdWString(L"c_char")))
    {
        stack.push(vm->GetAllocator().newObject(BuiltInTypes::c_char, BuiltInTypes::ClassType, false));
        return;
    }
    if((*constName) ==(QString::fromStdWString(L"c_asciiz")))
    {
        stack.push(vm->GetAllocator().newObject(BuiltInTypes::c_asciiz, BuiltInTypes::ClassType, false));
        return;
    }
    if((*constName) ==(QString::fromStdWString(L"c_wstr")))
    {
        stack.push(vm->GetAllocator().newObject(BuiltInTypes::c_wstr, BuiltInTypes::ClassType, false));
        return;
    }
    if((*constName) ==(QString::fromStdWString(L"c_ptr")))
    {
        stack.push(vm->GetAllocator().newObject(BuiltInTypes::c_ptr, BuiltInTypes::ClassType, false));
        return;
    }

    w->assert(false, ArgumentError, QString::fromStdWString(L"لا يوجد ثابت بهذا الاسم"));
}

void StringIsNumericProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    QString *s = popString(stack, w, vm);
    bool yep = true;
    for(int i=0; i<s->length(); i++)
    {
        QChar c = s->operator [](i);
        if(!c.isDigit())
            yep = false;
    }
    stack.push(vm->GetAllocator().newBool(yep));
}

void StringIsAlphabeticProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    QString *s = popString(stack, w, vm);
    bool yep = true;
    for(int i=0; i<s->length(); i++)
    {
        QChar c = s->operator [](i);
        if(!c.isLetter())
            yep = false;
    }
    stack.push(vm->GetAllocator().newBool(yep));
}

void TypeOfProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    Value *v = popValue(stack, w, vm);

    // We use a gcMonitor value of false since we don't want the GC
    // to collect class objects
    stack.push(vm->GetAllocator().newObject(v->type, BuiltInTypes::ClassType, false));
}

void TypeFromIdProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    QString *typeId = popString(stack, w, vm);
    Value *type = vm->GetType(*typeId);
    stack.push(type);
}

void AddressOfProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    Value *v = popValue(stack, w, vm);
    ffi_type *type;
    kalimat_to_ffi_type(v->type, type, vm);
    void *ptr = malloc(type->size);
    kalimat_to_ffi_value(v->type, v, type, ptr, vm);
    //todo:
    stack.push(vm->GetAllocator().newRaw(ptr, new PointerClass(v->type)));
}

void NewMapProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    stack.push(vm->GetAllocator().newMap());
}

void HasKeyProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    w->typeCheck(stack.top(), BuiltInTypes::MapType);
    Value *v = popValue(stack, w, vm);

    Value *key = popValue(stack, w, vm);

    VMap *m = v->unboxMap();
    VMError err;
    if(!m->keyCheck(key, err))
        throw err;
    bool result = (m->get(key) != NULL);
    stack.push(vm->GetAllocator().newBool(result));
}


struct FileBlob
{
    QFile *file;
    QTextStream *stream;
    ~FileBlob() { file->close(); delete file; delete stream;}
};

// TODO: use the helpers popXXX(...) functions instead of manually calling
// typecheck() and pop() in all external methods.
FileBlob *popFileBlob(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    verifyStackNotEmpty(stack, vm);
    w->typeCheck(stack.top(), BuiltInTypes::FileType);
    IObject *ob = stack.pop()->unboxObj();
    Value *rawFile = ob->getSlotValue("file_handle");
    w->typeCheck(rawFile, BuiltInTypes::RawFileType);
    void *fileObj = rawFile->unboxRaw();
    FileBlob *f = (FileBlob *) fileObj;
    return f;
}

Value *popValue(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    verifyStackNotEmpty(stack, vm);
    Value *v = stack.pop();
    return v;
}

QString *popString(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    verifyStackNotEmpty(stack, vm);
    w->typeCheck(stack.top(), BuiltInTypes::StringType);
    QString *s = stack.pop()->unboxStr();
    return s;
}

int popInt(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    verifyStackNotEmpty(stack, vm);
    w->typeCheck(stack.top(), BuiltInTypes::IntType);
    int i = stack.pop()->unboxInt();
    return i;
}

void *popRaw(QStack<Value *> &stack, RunWindow *w, VM *vm, IClass *type)
{
    verifyStackNotEmpty(stack, vm);
    w->typeCheck(stack.top(), type);
    void *ret = stack.pop()->unboxRaw();
    return ret;
}

bool popBool(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    verifyStackNotEmpty(stack, vm);
    w->typeCheck(stack.top(), BuiltInTypes::BoolType);
    bool b = stack.pop()->unboxBool();
    return b;
}

VArray *popArray(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    verifyStackNotEmpty(stack, vm);
    w->typeCheck(stack.top(), BuiltInTypes::ArrayType);
    VArray *arr = stack.pop()->unboxArray();
    return arr;
}

void DoFileWrite(QStack<Value *> &stack, RunWindow *w, VM *vm, bool newLine)
{
    FileBlob *f = popFileBlob(stack, w, vm);
    QString *s = popString(stack, w, vm);

    if(f->file == NULL)
        w->assert(false, ArgumentError, QString::fromStdWString(L"لا يمكن الكتابة في ملف مغلق"));
    if(newLine)
        *(f->stream) << *s << endl;
    else
        *(f->stream) << *s;
}

void FileWriteProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    DoFileWrite(stack, w, vm, false);
}

void FileWriteUsingWidthProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    FileBlob *f = popFileBlob(stack, w, vm);
    if(f->file == NULL)
        w->assert(false, ArgumentError, QString::fromStdWString(L"لا يمكن الكتابة في ملف مغلق"));
    QString *s = popString(stack, w, vm);
    int width = popInt(stack, w, vm);

    QString s2 = w->textLayer.formatStringUsingWidth(*s, width);
    *(f->stream) << s2;
}

void FileWriteLineProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    DoFileWrite(stack, w, vm, true);
}

void FileReadLineProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{

    FileBlob *f = popFileBlob(stack, w, vm);
    if(f->file == NULL)
        w->assert(false, ArgumentError, QString::fromStdWString(L"لا يمكن القراءة من ملف مغلق"));
    QString *s = new QString(f->stream->readLine());
    Value *v = vm->GetAllocator().newString(s);
    stack.push(v);
}

void FileEofProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    FileBlob *f = popFileBlob(stack, w, vm);
    if(f->file == NULL)
        w->assert(false, ArgumentError, QString::fromStdWString(L"لا يمكن التعامل مع ملف مغلق"));
    bool ret = f->stream->atEnd();
    Value *v = vm->GetAllocator().newBool(ret);
    stack.push(v);
}

void FileOpenProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    QString *fname = popString(stack, w, vm);
    *fname = w->ensureCompletePath(*fname);
    w->assert(QFile::exists(*fname), ArgumentError, QString::fromStdWString(L"محاولة فتح ملف غير موجود"));
    QFile *f = new QFile(*fname);
    bool ret = f->open(QIODevice::ReadOnly | QIODevice::Text);
    w->assert(ret, RuntimeError, QString::fromStdWString(L"لم ينجح فتح الملف"));
    QTextStream *stream = new QTextStream(f);
    FileBlob *blob = new FileBlob();
    blob->file = f;
    blob->stream = stream;
    Value *v = vm->GetAllocator().newObject(BuiltInTypes::FileType);
    v->v.objVal->setSlotValue("file_handle", vm->GetAllocator().newRaw(blob, BuiltInTypes::RawFileType));
    stack.push(v);
}

void FileCreateProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    QString *fname = popString(stack, w, vm);
    *fname = w->ensureCompletePath(*fname);
    QFile *f = new QFile(*fname);
    bool ret = f->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
    w->assert(ret, RuntimeError, QString::fromStdWString(L"لم ينجح فتح الملف"));
    QTextStream *stream = new QTextStream(f);
    FileBlob *blob = new FileBlob();
    blob->file = f;
    blob->stream = stream;
    Value *v = vm->GetAllocator().newObject(BuiltInTypes::FileType);
    v->v.objVal->setSlotValue("file_handle", vm->GetAllocator().newRaw(blob, BuiltInTypes::RawFileType));
    stack.push(v);
}

void FileAppendProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    QString *fname = popString(stack, w, vm);
    *fname = w->ensureCompletePath(*fname);
    QFile *f = new QFile(*fname);
    bool ret = f->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
    w->assert(ret, ArgumentError, QString::fromStdWString(L"لم ينجح فتح الملف"));
    Value *v = vm->GetAllocator().newObject(BuiltInTypes::FileType);
    QTextStream *stream = new QTextStream(f);
    FileBlob *blob = new FileBlob();
    blob->file = f;
    blob->stream = stream;
    v->v.objVal->setSlotValue("file_handle", vm->GetAllocator().newRaw(blob, BuiltInTypes::RawFileType));
    stack.push(v);
}

void FileCloseProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    // TODO: use popFileblob
    w->typeCheck(stack.top(), BuiltInTypes::FileType);
    IObject *ob = stack.pop()->unboxObj();
    Value *rawFile = ob->getSlotValue("file_handle");
    w->typeCheck(rawFile, BuiltInTypes::RawFileType);
    void *fileObj = rawFile->unboxRaw();
    FileBlob *f = (FileBlob *) fileObj;
    f->file->close();
    // TODO: memory leak if we comment the following line
    // but a segfault if we delete 'f' and the the kalimat code
    // tries to do some operation on the file :(
    delete f->file;
    delete f->stream;
    f->file = NULL;
}

Value *editAndReturn(Value *v, RunWindow *w, VM *vm);
void EditProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    Value *v = stack.pop();
    w->assert(v->tag == ObjectVal, ArgumentError, QString::fromStdWString(L"القيمة المرسلة لابد أن تكون كائناً"));
    v = editAndReturn(v, w, vm);
    stack.push(v);
}

void GetMainWindowProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    stack.push(vm->GetAllocator().newQObject(w));
}

void NewChannelProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    stack.push(vm->GetAllocator().newChannel());
}

void LoadLibraryProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    QString *libName = popString(stack, w, vm);
    // todo: will this leak?
    QLibrary *lib = new QLibrary(*libName);
    if(!lib->load())
        vm->signal(InternalError1, QString("Failed to load library '%1'").arg(*libName));

    Value *ret = vm->GetAllocator().newRaw(lib, BuiltInTypes::ExternalLibrary);
    stack.push(ret);
}

void GetProcAddressProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    void *libRaw = popRaw(stack, w, vm, BuiltInTypes::ExternalLibrary);
    QString *funcName = popString(stack, w, vm);
    // todo: invalid casts here will crash the VM
    QLibrary *lib = (QLibrary *) libRaw;
    // todo: all those conversion might be slow
    void * func = lib->resolve(funcName->toStdString().c_str());
    if(func == NULL)
    {
        vm->signal(InternalError1, QString("Cannot find function in called '%1' in external library %2")
                   .arg(*funcName).arg(lib->fileName()));
    }
    Value *ret = vm->GetAllocator().newRaw(func, BuiltInTypes::ExternalMethodType);
    stack.push(ret);
}

void InvokeForeignProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    void *funcPtr = popRaw(stack, w, vm, BuiltInTypes::ExternalMethodType);
    VArray *args = popArray(stack, w, vm);
    VArray *argTypes = NULL;
    bool guessArgTypes = false;
    if(stack.top()->type == BuiltInTypes::NullType)
    {
        guessArgTypes = true;
        stack.pop();
    }
    else
    {
        argTypes = popArray(stack, w, vm);
    }

    IClass *retType = (IClass *) stack.pop()->unboxObj();
    QVector<Value *> argz;
    QVector<IClass *> kargTypes;
    for(int i=0; i<args->count(); i++)
    {
        argz.append(args->Elements[i]);
        if(!guessArgTypes)
        {
            IClass *type = dynamic_cast<IClass *>(argTypes->Elements[i]->unboxObj());
            if(!type)
            {
                vm->signal(TypeError2, BuiltInTypes::ClassType->toString(), argTypes->Elements[i]->type->toString());
            }
            kargTypes.append(type);
        }
    }

    Value *ret = CallForeign(funcPtr, argz, retType, kargTypes, guessArgTypes, vm);
    if(ret)
        stack.push(ret);
    else
        stack.push(vm->GetAllocator().null());
}

void CurrentParseTreeProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    vm->DoPushGlobal("%parseTree");
    Value *v = vm->currentFrame()->OperandStack.pop();
    stack.push(v);
}

void MakeParserProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    QString *datum = popString(stack, w, vm);
    IClass *parserClass = dynamic_cast<IClass *>(vm->GetType(_ws(L"معرب"))->unboxObj());
    IObject *parser = parserClass->newValue(&vm->GetAllocator());
    parser->setSlotValue(_ws(L"موقع"), vm->GetAllocator().newInt(0));
    parser->setSlotValue(_ws(L"بيان"), vm->GetAllocator().newString(
                             new QString(*datum)));
    stack.push(vm->GetAllocator().newObject(parser, parserClass));
}

void setupChildren(QGridLayout *layout,Value *v, Reference *ref, QString label, int row, VM *vm)
{
    QCheckBox *cb;
    QGroupBox *qf;
    QScrollArea *sa;
    Object *obj;
    QGridLayout *vb;
    QLineEdit *le;

    int subRow;
    VArray *arr;
    switch(v->tag)
    {
    case Int:
    case Double:
    case Long:
    case StringVal:
        layout->addWidget(new QLabel(label), row, 0);
        le = new QLineEdit(v->toString());
        if(ref != NULL)
        {

            QObject::connect(le,
                       SIGNAL(textChanged(QString)),
                       new GUIEditWidgetHandler(
                           ref,
                           le,
                           vm),
                       SLOT(lineEditChanged()));
        }

        layout->addWidget(le, row, 1);
        break;
    case Boolean:
        cb = new QCheckBox();
        cb->setChecked(v->unboxBool());
        if(ref != NULL)
        {
            QObject::connect(cb,
                       SIGNAL(stateChanged(int)),
                       new GUIEditWidgetHandler(
                           ref,
                           cb,
                           vm),
                       SLOT(checkboxChanged(int)));
        }
        layout->addWidget(new QLabel(label), row, 0);
        layout->addWidget(cb, row, 1);
        break;
    case ObjectVal:
        qf = new QGroupBox(label);
        layout->addWidget(qf, row, 0, 1, 2);
        obj = dynamic_cast<Object *>(v->unboxObj());
        if(obj == NULL)
            break;
        vb = new QGridLayout();
        subRow = 0;
        for(QVector<QString>::iterator i = obj->slotNames.begin(); i!= obj->slotNames.end(); ++i)
        {
            setupChildren(vb, obj->_slots[*i], new FieldReference(obj, *i), *i, subRow++,vm);
        }
        qf->setLayout(vb);

        break;
    case ArrayVal:
        sa = new QScrollArea();
        vb = new QGridLayout();
        layout->addWidget(sa, row, 0, 1, 2);
        arr = v->unboxArray();
        vb->addWidget(new QLabel(label), 0, 0, 1, 2);
        for(int i=0; i<arr->count(); i++)
        {
            setupChildren(vb, arr->Elements[i], new ArrayReference(arr, i),QString("%1").arg(i+1), i+1, vm);
        }
        sa->setLayout(vb);
        sa->adjustSize();
        break;
    case NullVal:
    case RawVal:
    case RefVal:
    case MultiDimensionalArrayVal:
    case ChannelVal:
    case QObjectVal:
    case MapVal:
        break;
    }
}

Value *editAndReturn(Value *v, RunWindow *w, VM *vm)
{
    QDialog *dlg = new QDialog(w);
    dlg->setWindowTitle(QString::fromStdWString(L"تحرير %1").arg(v->type->getName()));
    QVBoxLayout *ly = new QVBoxLayout(dlg);

    QFrame *frame = new QFrame();
    QGridLayout *gl = new QGridLayout();
    setupChildren(gl, v, NULL, "", 0, vm);
    frame->setLayout(gl);
    ly->addWidget(frame);

    QPushButton *ok = new QPushButton(QString::fromStdWString(L"حسناً"));
    QPushButton *cancel = new QPushButton(QString::fromStdWString(L"الغاء"));
    ly->addWidget(ok);
    ly->addWidget(cancel);
    dlg->setLayout(ly);
    dlg->setLayoutDirection(Qt::RightToLeft);
    dlg->connect(ok, SIGNAL(clicked()), dlg, SLOT(accept()));
    dlg->connect(cancel, SIGNAL(clicked()), dlg, SLOT(reject()));
    bool ret = dlg->exec()== QDialog::Accepted;
    v = vm->GetAllocator().newBool(ret);

    return v;
}