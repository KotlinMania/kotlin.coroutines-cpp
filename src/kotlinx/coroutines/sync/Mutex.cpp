#include <string>
#include <optional>
#include "kotlinx/coroutines/sync/Mutex.hpp"
#include "kotlinx/coroutines/selects/Select.hpp"
#include "kotlinx/coroutines/sync/SemaphoreImpl.hpp" // Use generic impl

namespace kotlinx {
    namespace coroutines {
        namespace sync {
            using namespace kotlinx::coroutines::selects;

            struct MutexWaiter {
                virtual void invoke(std::exception_ptr) = 0;

                virtual ~MutexWaiter() = default;
            };

            // Symbol-like markers
            auto NO_OWNER = reinterpret_cast<void *>(0x100);
            auto ON_LOCK_ALREADY_LOCKED_BY_OWNER = reinterpret_cast<void *>(0x101);

            constexpr int TRY_LOCK_SUCCESS = 0;
            constexpr int TRY_LOCK_FAILED = 1;
            constexpr int TRY_LOCK_ALREADY_LOCKED_BY_OWNER = 2;

            constexpr int HOLDS_LOCK_UNLOCKED = 0;
            constexpr int HOLDS_LOCK_YES = 1;
            constexpr int HOLDS_LOCK_ANOTHER_OWNER = 2;


            class MutexImpl : public SemaphoreImpl, public Mutex {
            protected:
                std::atomic<void *> owner;
                OnCancellationConstructor on_select_cancellation_unlock_constructor;

            public:
                explicit MutexImpl(bool locked)
                    : SemaphoreImpl(1, locked ? 1 : 0),
                      owner(locked ? nullptr : NO_OWNER),
                      on_select_cancellation_unlock_constructor(
                          [](void *) {
                              // Stub
                          }
                      ) {
                }

                bool is_locked() const override {
                    return available.load() == 0;
                }

                bool holds_lock(void *owner_) override {
                    return holds_lock_impl(owner_) == HOLDS_LOCK_YES;
                }

            protected:
                int holds_lock_impl(void *owner_) {
                    while (true) {
                        if (!is_locked()) return HOLDS_LOCK_UNLOCKED;
                        void *cur_owner = owner.load();
                        if (cur_owner == NO_OWNER) continue;
                        return (cur_owner == owner_) ? HOLDS_LOCK_YES : HOLDS_LOCK_ANOTHER_OWNER;
                    }
                }

            public:
                void lock(void *owner_ = nullptr) override {
                    if (try_lock(owner_)) return;
                    lock_suspend(owner_);
                }

            private:
                void lock_suspend(void *owner_) {
                    // TODO: suspend function semantics not implemented
                    // acquire(cont_with_owner);
                }

            public:
                bool try_lock(void *owner_ = nullptr) override {
                    int result = try_lock_impl(owner_);
                    switch (result) {
                        case TRY_LOCK_SUCCESS: return true;
                        case TRY_LOCK_FAILED: return false;
                        case TRY_LOCK_ALREADY_LOCKED_BY_OWNER:
                            throw std::runtime_error("This mutex is already locked by the specified owner");
                        default: throw std::runtime_error("unexpected");
                    }
                }

            private:
                int try_lock_impl(void *owner_) {
                    while (true) {
                        if (try_acquire()) {
                            owner.store(owner_);
                            return TRY_LOCK_SUCCESS;
                        } else {
                            if (owner_ == nullptr) return TRY_LOCK_FAILED;
                            int holds = holds_lock_impl(owner_);
                            switch (holds) {
                                case HOLDS_LOCK_YES: return TRY_LOCK_ALREADY_LOCKED_BY_OWNER;
                                case HOLDS_LOCK_ANOTHER_OWNER: return TRY_LOCK_FAILED;
                                case HOLDS_LOCK_UNLOCKED: continue;
                            }
                        }
                    }
                }

            public:
                void unlock(void *owner_ = nullptr) override {
                    while (true) {
                        if (!is_locked()) throw std::runtime_error("This mutex is not locked");
                        void *cur_owner = owner.load();
                        if (cur_owner == NO_OWNER) continue;

                        if (!(cur_owner == owner_ || owner_ == nullptr)) {
                            throw std::runtime_error("This mutex is locked by different owner");
                        }
                        void *expected = cur_owner;
                        if (!owner.compare_exchange_strong(expected, NO_OWNER)) continue;
                        release();
                        return;
                    }
                }

                SelectClause2<void *, Mutex *> &get_on_lock() override {
                    static SelectClause2Impl<void *, Mutex *> clause(
                        this,
                        [](void *clause_obj, SelectInstance<void *> *select, void *param) {
                        },
                        [](void *clause_obj, void *param, void *result) -> void * { return nullptr; }
                    );
                    return clause;
                }

            protected:
                virtual void on_lock_reg_function(SelectInstance<void *> *select, void *owner_) {
                    if (owner_ != nullptr && holds_lock(owner_)) {
                        select->select_in_registration_phase(ON_LOCK_ALREADY_LOCKED_BY_OWNER);
                    }
                }

                virtual void *on_lock_process_result(void *owner_, void *result) {
                    if (result == ON_LOCK_ALREADY_LOCKED_BY_OWNER) {
                        throw std::runtime_error("This mutex is already locked by the specified owner");
                    }
                    return this;
                }

                class CancellableContinuationWithOwner : public MutexWaiter {
                public:
                    CancellableContinuation<void *> *cont;
                    void *owner;

                    CancellableContinuationWithOwner(
                        CancellableContinuation<void *> *cont_,
                        void *owner_
                    ) : cont(cont_), owner(owner_) {
                    }

                    virtual void invoke(std::exception_ptr) override {
                    }
                };

                template<typename Q>
                class SelectInstanceWithOwner : public SelectInstance<Q> {
                public:
                    SelectInstance<Q> *select;
                    void *owner;

                    SelectInstanceWithOwner(
                        SelectInstance<Q> *select_,
                        void *owner_
                    ) : select(select_), owner(owner_) {
                    }

                    bool try_select(void *clause_object, void *result) override {
                        bool success = select->try_select(clause_object, result);
                        if (success) {
                            // set owner
                        }
                        return success;
                    }

                    // Proxy methods
                    void dispose_on_completion(std::shared_ptr<DisposableHandle> handle) override {
                        select->dispose_on_completion(handle);
                    }

                    void select_in_registration_phase(void *internal_result) override {
                        select->select_in_registration_phase(internal_result);
                    }

                    std::shared_ptr<CancellableContinuation<Q> > get_continuation() override {
                        return select->get_continuation();
                    }
                };
            };

            Mutex *create_mutex(bool locked) {
                return new MutexImpl(locked);
            }
        } // namespace sync
    } // namespace coroutines
} // namespace kotlinx
