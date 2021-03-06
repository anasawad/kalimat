#ifndef VMRUNNERTHREAD_H
#define VMRUNNERTHREAD_H

#include <QThread>
#include <QMetaType>
#include <QWaitCondition>
#include <QMutex>

class VM;
class VMClient;
class RunWindow;

class VMRunthread : public QThread
{
    Q_OBJECT
    VM *vm;
    RunWindow *rw;
public:
    QMutex mutex;
    VMClient *client;
    bool runFast;
public:
    VMRunthread(VM *vm, RunWindow *rw);
    void run();
private:
    void normalRun();
    void fastRun();
};

#endif // VMRUNNERTHREAD_H
