//
// Created by ftd.wild on 2020/9/5.
//

#ifndef NETWORK_BASE_F_THREAD_H
#define NETWORK_BASE_F_THREAD_H

namespace ftdwild30 {

class Thread {
public:
    Thread();
    virtual ~Thread();

    int Start();

    void Stop();

private:
    void thread();

private:
    virtual int beforeStart() {return 0;};
    virtual void beforeStop() {};
    virtual int beforeThread() {return 0;};
    virtual int readThread() = 0;
    virtual int afterThread() {return 0;};
    virtual int afterStart() {return 0;};
    virtual void afterStop() {};

private:
    bool running_;
    bool quit_;
};

} // namespace ftdwild30

#endif //NETWORK_BASE_F_THREAD_H
