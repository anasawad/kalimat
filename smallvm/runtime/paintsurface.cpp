/**************************************************************************
**   The Kalimat programming language
**   Copyright 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "paintsurface.h"
#include <QFontDatabase>
#include <QTextDocument>
#include <QTextLayout>
#include <QAbstractTextDocumentLayout>
#include <QTextBlock>
#include <QDebug>

#include "../vm.h"

PaintSurface::PaintSurface(QSize size, QFont font)
    : image(size, QImage::Format_ARGB32_Premultiplied),
      finalImg(size, QImage::Format_ARGB32_Premultiplied)
{
    textColor = Qt::black;
    image.fill(qRgb(255,255,255));
    dirtyState = true; // to update first time
    showCoordinates = false;
    cursorTimerPoint = -1;

    // todo: this line is the only reference we have to VM
    // we need to remove this dependency and remove #include "vm.h"
    // from this file so that the PaintSurface class is not coupled with
    // SmallVM and can be used elsewhere
    locationFormatter = VM::argumentErrors.get(ArgErr::MouseLocationReport2);
}

bool PaintSurface::dirty()
{
    return dirtyState;
}

void PaintSurface::paint(QPainter &painter, TextLayer &textLayer, SpriteLayer &spriteLayer)
{
    if(textLayer.dirty() || spriteLayer.dirty() || this->dirty())
    {
        /*
        qDebug() << "updating!" << "Text: " << textLayer.dirty()
                 << " Sprites: " << spriteLayer.dirty()
                 << " Image: " << this->dirty();
        clock_t t1 = clock();
        //*/
        update(textLayer, spriteLayer);
        /*
        clock_t t2 = clock();
        qDebug() << "Drawing took " << TIME_DIFF_TO_SECONDS(t1, t2) << " seconds";
        //*/
        textLayer.updated();
        spriteLayer.updated();
        this->updated();
    }
    painter.drawImage(QPoint(0, 0), finalImg);
}

void PaintSurface::update(TextLayer &textLayer, SpriteLayer &spriteLayer)
{
    QPainter imgPainter(&finalImg);
    imgPainter.drawImage(QPoint(0,0), image);
    //imgPainter.fillRect(image.rect(), Qt::white);
    drawTextLayer(imgPainter, textLayer);
    drawSpriteLayer(imgPainter, spriteLayer);

    const int shiftDist = 15;
    if(showCoordinates)
    {
        int x1 =shiftDist, y1 =shiftDist,
                x2 = this->image.width() - shiftDist, y2 = shiftDist;
        TX(x1);
        TX(x2);
        imgPainter.setPen(QColor::fromRgb(255,0,0));
        imgPainter.drawLine(x1, y1, x2, y2);
        imgPainter.drawLine(x2, y2, x2+5, y2-5);
        imgPainter.drawLine(x2, y2, x2+5, y2+5);

        x2 = shiftDist;
        TX(x2);
        y2 = this->image.height() - shiftDist;
        imgPainter.drawLine(x1, y1, x2, y2);
        imgPainter.drawLine(x2, y2, x2-5, y2-5);
        imgPainter.drawLine(x2, y2, x2+5, y2-5);

        QString loc = locationFormatter.arg(mouseLocationForDemo.x()).arg(mouseLocationForDemo.y());
        int locX = 230;
        int locY = 20 + shiftDist;
        int w = imgPainter.fontMetrics().width(loc);
        TX(locX, w);
        imgPainter.drawText(locX, locY, loc);
    }
}

void PaintSurface::drawTextLayer(QPainter &imgPainter, TextLayer &textLayer)
{
    const int timeUnitsPerSec = 1000000;
    if(textLayer.inputState())
    {
        bool drawCursor = false;
        if(cursorTimerPoint == -1)
        {
            cursorTimerPoint = get_time();
            cursorTimeSoFar = 0.0;
        }
        else
        {
            long t = get_time();
            cursorTimeSoFar += t - cursorTimerPoint;
            cursorTimerPoint = t;
        }
        if(cursorTimeSoFar < (timeUnitsPerSec / 2))
        {
            drawCursor = true;
        }
        else if(cursorTimeSoFar > timeUnitsPerSec)
        {
            cursorTimeSoFar = 0;
        }

        textLayer.updateStrip(textLayer.cursorLine(), drawCursor);
    }

    /*
    qreal height = textLayer.stripHeight();
    for(int i=0; i < visibleTextLines; ++i)
    {
        //imgPainter.drawImage(0, i * height, textLayer.strip(i));
        imgPainter.drawPixmap(0, i * height, textLayer.strip(i));
    }
    */
    imgPainter.drawPixmap(0, 0, textLayer.getImage());
}

void PaintSurface::drawSpriteLayer(QPainter &imgPainter, SpriteLayer &spriteLayer)
{
    QPainter::CompositionMode oldMode = imgPainter.compositionMode();
    for(int i=0; i < spriteLayer.sprites().count(); ++i)
    {
        Sprite *s = spriteLayer.sprites()[i];
        if(s->visible)
        {
            //imgPainter.setCompositionMode(QPainter::RasterOp_SourceAndDestination);
            //imgPainter.drawPixmap(s->location, s->mask);
            //imgPainter.setCompositionMode(QPainter::RasterOp_SourceOrDestination);
            int x = s->location.x();
            int y = s->location.y();
            TX(x, s->image.width());

            imgPainter.drawPixmap(x, y, s->image);
        }
    }
    imgPainter.setCompositionMode(oldMode);
}

const QImage *PaintSurface::GetImage()
{
    return &this->image;
}

QImage *PaintSurface::GetImageForWriting()
{
    dirtyState = true;
    return &this->image;
}

QColor PaintSurface::GetColor(int color)
{
    switch(color)
    {
    case 0:
        return QColor::fromRgb(0,0,0);
    case 1:
        return QColor::fromRgb(255,145,72);
    case 2:
        return QColor::fromRgb(36,218,36);
    case 3:
        return QColor::fromRgb(109,255,109);
    case 4:
        return QColor::fromRgb(36,35,255);
    case 5:
        return QColor::fromRgb(72,109,255);
    case 6:
        return QColor::fromRgb(182,36,36);
    case 7:
        return QColor::fromRgb(72,182,255);
    case 8:
        return QColor::fromRgb(255,36,36);
    case 9:
        return QColor::fromRgb(255,109,109);
    case 10:
        return QColor::fromRgb(218,218,36);
    case 11:
        return QColor::fromRgb(218,218,145);
    case 12:
        return QColor::fromRgb(36,145,36);
    case 13:
        return QColor::fromRgb(218,72,182);
    case 14:
        return QColor::fromRgb(182,182,182);
    case 15:
        return QColor::fromRgb(255,255,255);
    default:
        return Qt::white;
    }
}

int PaintSurface::colorConstant(QColor color)
{
    int minDiff = -1;
    int minIndex = -1;
    for(int i=0; i<16; i++)
    {
        QColor base = GetColor(i);
        const int rdiff = color.red() - base.red();
        const int gdiff = color.green() - base.green();
        const int bdiff = color.blue() - base.blue();
        const int diff = rdiff*rdiff + gdiff*gdiff + bdiff * bdiff;
        if(i==0)
            minDiff = diff;
        else if(diff < minDiff)
        {
            minDiff = diff;
            minIndex = i;
        }
    }
    return minIndex;
}

void PaintSurface::zoom(int x1, int y1, int x2, int y2)
{

}

void PaintSurface::clearImage()
{
    QPainter p(GetImageForWriting());
    p.fillRect(0,0,GetImage()->width(), GetImage()->height(), Qt::white);
}

void PaintSurface::TX(int &x)
{
#ifndef ENGLISH_PL
    x = (this->image.width()-1)-x;
#endif
}

void PaintSurface::TX(int &x, int w)
{
#ifndef ENGLISH_PL
    x = (this->image.width()-1)-x;
    x-= w;
#endif
}

void PaintSurface::setTextColor(QColor color)
{
    textColor = color;
}

void PaintSurface::resize(int width, int height)
{
    if (width > image.width() || height > image.height())
    {
        int newWidth = qMax(width + 128, image.width());
        int newHeight = qMax(height + 128, image.height());
        resizeImage(&image, QSize(newWidth, newHeight));
        finalImg = QImage(image.size(), finalImg.format());
    }
}

void PaintSurface::resizeImage(QImage *image, const QSize &newSize)
{
     if (image->size() == newSize)
         return;
     QImage newImage(newSize, image->format());
     newImage.fill(qRgb(255, 255, 255));
     QPainter painter(&newImage);
     painter.drawImage(QPoint(0, 0), *image);
     *image = newImage;
}
