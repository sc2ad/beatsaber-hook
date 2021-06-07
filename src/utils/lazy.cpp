#include "../../shared/utils/lazy.hpp"

#include <cassert>
#include <condition_variable>
#include <mutex>

constexpr std::uintptr_t UNINIT = 0x0;
constexpr std::uintptr_t RUNNING = 0x1;
constexpr std::uintptr_t INIT = 0x2;

constexpr std::uintptr_t STATE_MASK = 0b11;

class Waiter {
    std::mutex lock;
    std::condition_variable cv;
    std::atomic_bool signaled;

  public:
    Waiter* next;

    Waiter(const std::uintptr_t current_state)
        : lock(), cv(), signaled(false),
          next(reinterpret_cast<Waiter*>(current_state & ~STATE_MASK)) {
        // We need the alignment to be at least 4 to fit our state in the two
        // lower bits of Waiter pointers. This should be the case 99% of the
        // time.
        static_assert(alignof(Waiter) >= 4);
    }

    void wait() {
        std::unique_lock<std::mutex> lock(this->lock);
        while (!this->signaled.load(std::memory_order_acquire)) {
            cv.wait(lock);
        }
    }

    void notify() {
        this->signaled.store(true, std::memory_order_release);
        this->cv.notify_one();
    }
};

class WaiterQueue {
    // This is a reference to the state_and_queue of Once<T>. Since the
    // WaiterQueue is a short lived value which only ever exists on the stack,
    // having a reference member is fine.
    std::atomic_uintptr_t& state_and_queue;

  public:
    // Value the state will be reset to when the destructor is run. This doesn't
    // need to be atomic as it is only ever accessed by the initializing thread.
    std::uintptr_t set_state_on_destroy_to;

    constexpr WaiterQueue(std::atomic_uintptr_t& state_and_queue_,
                          std::uintptr_t set_state_on_destroy_to_)
        : state_and_queue(state_and_queue_),
          set_state_on_destroy_to(set_state_on_destroy_to_) {}

    ~WaiterQueue() {
        // Grab the queue and swap it with the value it should have when
        // destroyed.
        auto const state_and_queue = this->state_and_queue.exchange(
            this->set_state_on_destroy_to, std::memory_order_acq_rel);

        assert((state_and_queue & STATE_MASK) == RUNNING);

        // Iterate over waiters in the queue and wake them up
        auto next = reinterpret_cast<Waiter*>(state_and_queue & ~STATE_MASK);
        while (next != nullptr) {
            auto tmp = next->next;
            std::swap(next, tmp);
            tmp->notify();
        }
    }
};

template<typename T>
void Once<T>::initialize(std::function<T()> const& initializer) {
    auto state_and_queue =
        this->state_and_queue.load(std::memory_order_acquire);
    while (true) {
        if (state_and_queue == INIT) [[likely]] {
            return;
        } else if (state_and_queue == UNINIT) /* initialisation */ {
            // Set the current state to RUNNING, but only if another thread
            // hasn't already since the previous load. If that's the case,
            // we keep looping (the value of state_and_queue will be
            // updated).
            auto const exchanged =
                this->state_and_queue.compare_exchange_strong(
                    state_and_queue, RUNNING,
                    std::memory_order_acquire);
            if (!exchanged) {
                continue;
            }

            // Create the waiter queue, telling it to set the state to
            // UNINIT when destructed.
            WaiterQueue waiter_queue(this->state_and_queue,
                                             UNINIT);
            // Initialize the value. If the initializer throws, the waiter
            // queue destructor runs and the state is reset to UNINIT.
            this->value = initializer();

            // If this is reached the initializer has run successfully and
            // the value is initalized, so we can tell the waiter queue to
            // set the state to INIT when destructed (which happens
            // immediately cause we return).
            waiter_queue.set_state_on_destroy_to = INIT;
            return;
        } else [[unlikely]] /* waiting */ {
            assert((state_and_queue & STATE_MASK) ==
                   RUNNING);

            while ((state_and_queue & STATE_MASK) ==
                   RUNNING) {
                // Construct a waiter on the current thread's stack, with
                // its next pointer pointing to the current head of the
                // waiter queue.
                Waiter node(state_and_queue);
                auto const me = reinterpret_cast<uintptr_t>(&node);

                // Add our waiter to the queue, but only if the current head
                // has not been changed by another thread (if we don't check
                // we might leak a waiter and deadlock its thread). It's
                // fine to reference our stack local waiter from the queue
                // because we will be blocking the thread (and therefore
                // keeping our stack alive) until the queue itself is
                // destroyed.
                auto const exchanged =
                    this->state_and_queue.compare_exchange_strong(
                        state_and_queue, me | RUNNING,
                        std::memory_order_release,
                        std::memory_order_relaxed);
                if (!exchanged) {
                    continue;
                }

                node.wait();
                break;
            }

            state_and_queue =
                this->state_and_queue.load(std::memory_order_acquire);
        }
    }
}
