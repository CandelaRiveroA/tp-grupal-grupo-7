#include "common_worker.h"

Worker::Worker(): failure(nullptr) {}

void Worker::run() {
    try {
        execute();
    } catch (const ClosedQueue&) {
    } catch (...) {
        failure = std::current_exception();
    }
}

bool Worker::failed() const { return failure != nullptr; }

std::exception_ptr Worker::exception() const { return failure; }
