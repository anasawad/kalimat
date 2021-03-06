/**************************************************************************
**   The Kalimat programming language
**   Copyright 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef PAINTSURFACE_H
#define PAINTSURFACE_H

#include <QPainter>

#ifndef TEXTLAYER_H
    #include "textlayer.h"
#endif

#ifndef SPRITELAYER_H
    #include "spritelayer.h"
#endif

#include <time.h>

class QTextDocument;
class PaintSurface
{
    QImage image;
    QImage finalImg;
    QColor textColor;
    bool dirtyState;
    QString locationFormatter;

    long cursorTimerPoint;
    double cursorTimeSoFar;
public:
    // We have a special 'demo coordinates' mode in Kalimat
    // to teach children about the x/y coordinate system
    // by drawing the axes and showing mouse position if this
    // mode was activated

    bool showCoordinates;
    QPoint mouseLocationForDemo;

public:
    PaintSurface(QSize size, QFont font);
    bool dirty();
    void updated() { dirtyState = false;}
    void paint(QPainter &painter, TextLayer &textLayer, SpriteLayer &spriteLayer);
    void update(TextLayer &textLayer, SpriteLayer &spriteLayer);
    void TX(int &);
    void TX(int &x, int width);

    void zoom(int x1, int y1, int x2, int y2);
    void clearImage();
    const QImage *GetImage();
    QImage *GetImageForWriting();
    QColor GetColor(int color);
    int colorConstant(QColor color);
    void setTextColor(QColor);
    void resize(int width, int height);
    void resizeImage(QImage *image, const QSize &newSize);

    void adjustFontForNumberOfLines(int n);
    void drawTextLayer(QPainter &imgPainter, TextLayer &layer);
    void fastDrawTextLayer(QPainter &imgPainter, TextLayer &layer);
    void drawSpriteLayer(QPainter &imgPainter, SpriteLayer &layer);
};

#endif // PAINTSURFACE_H
