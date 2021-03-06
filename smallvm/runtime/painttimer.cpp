/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "painttimer.h"
#include "../utils.h"

const int TIME_UNITS_PER_SEC = 1000000;
PaintTimer::PaintTimer(int frameRate)
{
    minTime = TIME_UNITS_PER_SEC / frameRate;
    first = true;
}

bool PaintTimer::canUpdateNow()
{
    if(first)
    {
        first = false;
        lastTimeStamp = get_time();
        elapsedTime = 0;
        return true;
    }
    else
    {
        long ts = get_time();
        elapsedTime += (ts - lastTimeStamp);
        lastTimeStamp = ts;
        if(elapsedTime >= minTime)
        {
            elapsedTime = 0;
            return true;
        }
        else
        {
            return false;
        }
    }
}
