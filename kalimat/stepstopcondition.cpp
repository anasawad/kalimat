#include "stepstopcondition.h"
#include "mainwindow.h"

void SingleStepCondition::stopNow(Process *proc)
{
    if(proc->isFinished())
        return;
    Frame *frame = proc->currentFrame();
    const Instruction &i = frame->currentMethod->Get(frame->ip);
    if(mw->PositionInfo.contains(i.extra))
    {
        CodePosition &pos = mw->PositionInfo[i.extra];
        if(pos.ast->getPos().Line != startingLine
                || pos.doc != this->doc
                || frame != this->startingFrame)
        {
            proc->Break(BreakSource::FromStepping);
            return;
        }
    }
}

void StepOverCondition::stopNow(Process *proc)
{
    if(proc->isFinished())
        return;
    Frame *frame = proc->currentFrame();
    const Instruction &i = frame->currentMethod->Get(frame->ip);
    if(mw->PositionInfo.contains(i.extra))
    {
        CodePosition &pos = mw->PositionInfo[i.extra];
        if(pos.ast->getPos().Line != startingLine
                && pos.doc == this->doc
                && frame == this->startingFrame)
        {
            proc->Break(BreakSource::FromStepping);
            return;
        }
    }
}

NullaryStepStopCondition *NullaryStepStopCondition::instance()
{
    static NullaryStepStopCondition *inst = NULL;
    if(!inst)
    {
        inst = new NullaryStepStopCondition();
    }
    return inst;
}
