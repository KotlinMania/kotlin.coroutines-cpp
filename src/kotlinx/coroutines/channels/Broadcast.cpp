#include <memory>
#include <functional>
#include <exception>
#include "kotlinx/coroutines/Unit.hpp"
#include "kotlinx/coroutines/AbstractCoroutine.hpp"
#include "kotlinx/coroutines/channels/BroadcastChannel.hpp"
#include "kotlinx/coroutines/channels/ProducerScope.hpp"

namespace kotlinx {
    namespace coroutines {
        namespace channels {
            template<typename E>
            class BroadcastCoroutine : public AbstractCoroutine<Unit>, public ProducerScope<E> {
            public:
                BroadcastChannel<E> *_channel;

                BroadcastCoroutine(
                    std::shared_ptr<CoroutineContext> context,
                    BroadcastChannel<E> *channel,
                    bool active
                ) : AbstractCoroutine<Unit>(context, true, active), _channel(channel) {
                }

                virtual ~BroadcastCoroutine() = default;

                bool is_active() const override { return AbstractCoroutine<Unit>::is_active(); }

                // AbstractCoroutine overrides
                void on_completed(Unit value) override {
                    // _channel->close(); // TODO
                }

                void on_cancelled(std::exception_ptr cause, bool handled) override {
                    // _channel->close(cause); // TODO
                }

                // ProducerScope overrides (SendChannel)
                bool is_closed_for_send() const override { return _channel->is_closed_for_send(); }
                void send(E element) override { _channel->send(element); }
                ChannelResult<void> try_send(E element) override { return _channel->try_send(element); }
                bool close(std::exception_ptr cause = nullptr) override { return _channel->close(cause); }

                void invoke_on_close(std::function<void(std::exception_ptr)> handler) override {
                    _channel->invoke_on_close(handler);
                }

                // ProducerScope specific
                SendChannel<E> *get_channel() override { return _channel; }
            };

            // LazyBroadcastCoroutine stub
            template<typename E>
            class LazyBroadcastCoroutine : public BroadcastCoroutine<E> {
            public:
                LazyBroadcastCoroutine(
                    std::shared_ptr<CoroutineContext> context,
                    BroadcastChannel<E> *channel,
                    std::function<void(ProducerScope<E> *)> block
                ) : BroadcastCoroutine<E>(context, channel, false) {
                }

                // Stub overrides
            };
        } // namespace channels
    } // namespace coroutines
} // namespace kotlinx