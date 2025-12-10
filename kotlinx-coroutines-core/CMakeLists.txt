cmake_minimum_required(VERSION 3.16)
project(kotlinx-coroutines-core VERSION 1.0.0 LANGUAGES CXX)

# Enable C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Create the main kotlinx-coroutines-core library
set(CORE_SOURCES
    common/src/AbstractCoroutine.cpp
    common/src/Annotations.cpp
    common/src/Await.cpp
    common/src/Builders.common.cpp
    common/src/CancellableContinuation.cpp
    common/src/CancellableContinuationImpl.cpp
    common/src/CloseableCoroutineDispatcher.cpp
    common/src/CompletableDeferred.cpp

    common/src/CompletionHandler.common.cpp
    common/src/CompletionState.cpp
    common/src/CoroutineContext.common.cpp
    common/src/CoroutineDispatcher.cpp
    common/src/CoroutineExceptionHandler.cpp
    common/src/CoroutineName.cpp
    common/src/CoroutineScope.cpp
    common/src/CoroutineStart.cpp
    common/src/Debug.common.cpp
    common/src/Deferred.cpp
    common/src/Delay.cpp
    common/src/Dispatchers.common.cpp
    common/src/Dispatchers.cpp
    common/src/EventLoop.common.cpp
    common/src/Exceptions.common.cpp
    common/src/Guidance.cpp
    common/src/Job.cpp
    common/src/JobImpl.cpp
    common/src/JobSupport.cpp
    common/src/MainCoroutineDispatcher.cpp
    common/src/NonCancellable.cpp
    common/src/Runnable.common.cpp
    common/src/SchedulerTask.common.cpp
    common/src/Supervisor.cpp
    common/src/Timeout.cpp
    common/src/Unconfined.cpp
    common/src/Waiter.cpp
    common/src/Yield.cpp
    common/src/channels/Broadcast.cpp
    common/src/channels/BroadcastChannel.cpp
    common/src/channels/BufferedChannel.cpp
    common/src/channels/Channel.cpp
    common/src/channels/ChannelCoroutine.cpp
    common/src/channels/Channels.common.cpp
    common/src/channels/ConflatedBufferedChannel.cpp
    common/src/channels/Deprecated.cpp
    common/src/channels/Produce.cpp
    common/src/flow/Builders.cpp
    common/src/flow/Channels.cpp
    common/src/flow/Flow.cpp
    common/src/flow/FlowCollector.cpp
    common/src/flow/Migration.cpp
    common/src/flow/SharedFlow.cpp
    common/src/flow/SharingStarted.cpp
    common/src/flow/StateFlow.cpp
    common/src/flow/internal/AbstractSharedFlow.cpp
    common/src/flow/internal/ChannelFlow.cpp
    common/src/flow/internal/Combine.cpp
    common/src/flow/internal/FlowCoroutine.cpp
    common/src/flow/internal/FlowExceptions.common.cpp
    common/src/flow/internal/Merge.cpp
    common/src/flow/internal/NopCollector.cpp
    common/src/flow/internal/NullSurrogate.cpp
    common/src/flow/internal/SafeCollector.common.cpp
    common/src/flow/internal/SafeCollector.cpp
    common/src/flow/internal/SendingCollector.cpp
    common/src/flow/operators/Context.cpp
    common/src/flow/operators/Delay.cpp
    common/src/flow/operators/Distinct.cpp
    common/src/flow/operators/Emitters.cpp
    common/src/flow/operators/Errors.cpp
    common/src/flow/operators/Limit.cpp
    common/src/flow/operators/Lint.cpp
    common/src/flow/operators/Merge.cpp
    common/src/flow/operators/Share.cpp
    common/src/flow/operators/Transform.cpp
    common/src/flow/operators/Zip.cpp
    common/src/flow/terminal/Collect.cpp
    common/src/flow/terminal/Collection.cpp
    common/src/flow/terminal/Count.cpp
    common/src/flow/terminal/Logic.cpp
    common/src/flow/terminal/Reduce.cpp
    common/src/internal/Concurrent.common.cpp
    common/src/internal/ConcurrentLinkedList.cpp
    common/src/internal/CoroutineExceptionHandlerImpl.common.cpp
    common/src/internal/DispatchedContinuation.cpp
    common/src/internal/DispatchedTask.cpp
    common/src/internal/InlineList.cpp
    common/src/internal/InternalAnnotations.common.cpp
    common/src/internal/LimitedDispatcher.cpp
    common/src/internal/LocalAtomics.common.cpp
    common/src/internal/LockFreeLinkedList.common.cpp
    common/src/internal/LockFreeTaskQueue.cpp
    common/src/internal/MainDispatcherFactory.cpp
    common/src/internal/NamedDispatcher.cpp
    common/src/internal/OnUndeliveredElement.cpp
    common/src/internal/ProbesSupport.common.cpp
    common/src/internal/Scopes.cpp
    common/src/internal/StackTraceRecovery.common.cpp
    common/src/internal/Symbol.cpp
    common/src/internal/Synchronized.common.cpp
    common/src/internal/SystemProps.common.cpp
    common/src/internal/ThreadContext.common.cpp
    common/src/internal/ThreadLocal.common.cpp
    common/src/internal/ThreadSafeHeap.cpp
    common/src/intrinsics/Cancellable.cpp
    common/src/intrinsics/Undispatched.cpp
    common/src/selects/OnTimeout.cpp
    common/src/selects/Select.cpp
    common/src/selects/SelectOld.cpp
    common/src/selects/SelectUnbiased.cpp
    common/src/selects/WhileSelect.cpp
    common/src/sync/Mutex.cpp
    common/src/sync/Semaphore.cpp
    concurrent/src/Builders.concurrent.cpp
    concurrent/src/Dispatchers.cpp
    concurrent/src/MultithreadedDispatchers.common.cpp
    concurrent/src/channels/Channels.cpp
    concurrent/src/internal/LockFreeLinkedList.cpp
    concurrent/src/internal/OnDemandAllocatingPool.cpp
    native/src/Builders.cpp
    native/src/CloseableCoroutineDispatcher.cpp
    native/src/CoroutineContext.cpp
    native/src/Debug.cpp
    native/src/Dispatchers.cpp
    native/src/EventLoop.cpp
    native/src/Exceptions.cpp
    native/src/MultithreadedDispatchers.cpp
    native/src/Runnable.cpp
    native/src/SchedulerTask.cpp
    native/src/flow/internal/FlowExceptions.cpp
    native/src/flow/internal/SafeCollector.cpp
    native/src/internal/Concurrent.cpp
    native/src/internal/CopyOnWriteList.cpp
    native/src/internal/CoroutineExceptionHandlerImpl.cpp
    native/src/internal/LocalAtomics.cpp
    native/src/internal/ProbesSupport.cpp
    native/src/internal/StackTraceRecovery.cpp
    native/src/internal/Synchronized.cpp
    native/src/internal/SystemProps.cpp
    native/src/internal/ThreadContext.cpp
    native/src/internal/ThreadLocal.cpp
    nativeDarwin/src/Dispatchers.cpp
)

# Create the core library
add_library(kotlinx-coroutines-core ${CORE_SOURCES})

# Set include directories for this target
target_include_directories(kotlinx-coroutines-core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/common/src
        ${CMAKE_CURRENT_SOURCE_DIR}/common/src/channels
        ${CMAKE_CURRENT_SOURCE_DIR}/common/src/sync
        ${CMAKE_CURRENT_SOURCE_DIR}/common/src/flow
        ${CMAKE_CURRENT_SOURCE_DIR}/common/src/internal
        ${CMAKE_CURRENT_SOURCE_DIR}/common/src/selects
        ${CMAKE_CURRENT_SOURCE_DIR}/native/src
        ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/src
        ${CMAKE_CURRENT_SOURCE_DIR}/nativeDarwin/src
)

# Link against threading libraries
find_package(Threads REQUIRED)
target_link_libraries(kotlinx-coroutines-core PUBLIC Threads::Threads)

# Set library properties
set_target_properties(kotlinx-coroutines-core PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Install the library
install(TARGETS kotlinx-coroutines-core
    EXPORT kotlinx-coroutines-core-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

# Install the export
install(EXPORT kotlinx-coroutines-core-targets
    FILE kotlinx-coroutines-core-targets.cmake
    NAMESPACE kotlinx::
    DESTINATION lib/cmake/kotlinx-coroutines-core
)