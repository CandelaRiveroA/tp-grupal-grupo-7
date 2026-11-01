#include "common_worker.h"

Worker::Worker(): failure(nullptr), has_failed(false) {}

void Worker::run() {
    try {
        execute();
    } catch (const ClosedQueue&) {
    } catch (...) {
        failure = std::current_exception();
        has_failed = true;
    }
}

bool Worker::failed() const { return has_failed; }

std::exception_ptr Worker::exception() const { return failure; }
