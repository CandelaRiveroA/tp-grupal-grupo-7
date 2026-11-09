#ifndef COMMON_WORKER_H
#define COMMON_WORKER_H

#include <atomic>
#include <exception>

#include "common_queue.h"
#include "common_thread.h"

class Worker: public Thread {
private:
    std::exception_ptr failure;
    std::atomic<bool> has_failed;

protected:
    virtual void execute() = 0;

public:
    Worker();

    void run() final;

    bool failed() const;
    std::exception_ptr exception() const;
};

#endif
