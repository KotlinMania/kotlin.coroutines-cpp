cmake_minimum_required(VERSION 3.16)
project(kotlinx-coroutines-core VERSION 1.0.0 LANGUAGES CXX)

# Enable C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Create the main kotlinx-coroutines-core library
set(CORE_SOURCES
    kotlinx/coroutines/common/src/AbstractCoroutine.cpp
    kotlinx/coroutines/common/src/Annotations.cpp
    kotlinx/coroutines/common/src/Await.cpp
    kotlinx/coroutines/common/src/Builders.common.cpp
    kotlinx/coroutines/common/src/CancellableContinuation.cpp
    kotlinx/coroutines/common/src/CancellableContinuationImpl.cpp
    kotlinx/coroutines/common/src/CloseableCoroutineDispatcher.cpp
    kotlinx/coroutines/common/src/Coroutine.cpp
    kotlinx/coroutines/common/src/CoroutineContext.common.cpp
    kotlinx/coroutines/common/src/CoroutineDispatcher.cpp
    kotlinx/coroutines/common/src/CoroutineExceptionHandler.cpp
    kotlinx/coroutines/common/src/CoroutineName.cpp
    kotlinx/coroutines/common/src/CoroutineScope.cpp
    kotlinx/coroutines/common/src/CoroutineStart.cpp
    kotlinx/coroutines/common/src/Debug.common.cpp
    kotlinx/coroutines/common/src/Deferred.cpp
    kotlinx/coroutines/common/src/Delay.cpp
    kotlinx/coroutines/common/src/Dispatchers.common.cpp
    kotlinx/coroutines/common/src/Dispatchers.cpp
    kotlinx/coroutines/common/src/EventLoop.common.cpp
    kotlinx/coroutines/common/src/Exceptions.common.cpp
    kotlinx/coroutines/common/src/Guidance.cpp
    kotlinx/coroutines/common/src/Job.cpp
    kotlinx/coroutines/common/src/JobImpl.cpp
    kotlinx/coroutines/common/src/JobSupport.cpp
    kotlinx/coroutines/common/src/MainCoroutineDispatcher.cpp
    kotlinx/coroutines/common/src/NonCancellable.cpp
    kotlinx/coroutines/common/src/Runnable.common.cpp
    kotlinx/coroutines/common/src/SchedulerTask.common.cpp
    kotlinx/coroutines/common/src/Supervisor.cpp
    kotlinx/coroutines/common/src/Timeout.cpp
    kotlinx/coroutines/common/src/Unconfined.cpp
    kotlinx/coroutines/common/src/Waiter.cpp
    kotlinx/coroutines/common/src/Yield.cpp
    kotlinx/coroutines/common/src/channels/Broadcast.cpp
    kotlinx/coroutines/common/src/channels/BroadcastChannel.cpp
    kotlinx/coroutines/common/src/channels/BufferedChannel.cpp
    kotlinx/coroutines/common/src/channels/Channel.cpp
    kotlinx/coroutines/common/src/channels/ChannelCoroutine.cpp
    kotlinx/coroutines/common/src/channels/Channels.common.cpp
    kotlinx/coroutines/common/src/channels/ConflatedBufferedChannel.cpp
    kotlinx/coroutines/common/src/channels/Deprecated.cpp
    kotlinx/coroutines/common/src/channels/Produce.cpp
    kotlinx/coroutines/common/src/flow/Builders.cpp
    kotlinx/coroutines/common/src/flow/Channels.cpp
    kotlinx/coroutines/common/src/flow/Flow.cpp
    kotlinx/coroutines/common/src/flow/FlowCollector.cpp
    kotlinx/coroutines/common/src/flow/Migration.cpp
    kotlinx/coroutines/common/src/flow/SharedFlow.cpp
    kotlinx/coroutines/common/src/flow/SharingStarted.cpp
    kotlinx/coroutines/common/src/flow/StateFlow.cpp
    kotlinx/coroutines/common/src/flow/internal/AbstractSharedFlow.cpp
    kotlinx/coroutines/common/src/flow/internal/ChannelFlow.cpp
    kotlinx/coroutines/common/src/flow/internal/Combine.cpp
    kotlinx/coroutines/common/src/flow/internal/FlowCoroutine.cpp
    kotlinx/coroutines/common/src/flow/internal/FlowExceptions.common.cpp
    kotlinx/coroutines/common/src/flow/internal/Merge.cpp
    kotlinx/coroutines/common/src/flow/internal/NopCollector.cpp
    kotlinx/coroutines/common/src/flow/internal/NullSurrogate.cpp
    kotlinx/coroutines/common/src/flow/internal/SafeCollector.common.cpp
    kotlinx/coroutines/common/src/flow/internal/SafeCollector.cpp
    kotlinx/coroutines/common/src/flow/internal/SendingCollector.cpp
    kotlinx/coroutines/common/src/flow/operators/Context.cpp
    kotlinx/coroutines/common/src/flow/operators/Delay.cpp
    kotlinx/coroutines/common/src/flow/operators/Distinct.cpp
    kotlinx/coroutines/common/src/flow/operators/Emitters.cpp
    kotlinx/coroutines/common/src/flow/operators/Errors.cpp
    kotlinx/coroutines/common/src/flow/operators/Limit.cpp
    kotlinx/coroutines/common/src/flow/operators/Lint.cpp
    kotlinx/coroutines/common/src/flow/operators/Merge.cpp
    kotlinx/coroutines/common/src/flow/operators/Share.cpp
    kotlinx/coroutines/common/src/flow/operators/Transform.cpp
    kotlinx/coroutines/common/src/flow/operators/Zip.cpp
    kotlinx/coroutines/common/src/flow/terminal/Collect.cpp
    kotlinx/coroutines/common/src/flow/terminal/Collection.cpp
    kotlinx/coroutines/common/src/flow/terminal/Count.cpp
    kotlinx/coroutines/common/src/flow/terminal/Logic.cpp
    kotlinx/coroutines/common/src/flow/terminal/Reduce.cpp
    kotlinx/coroutines/common/src/internal/Concurrent.common.cpp
    kotlinx/coroutines/common/src/internal/ConcurrentLinkedList.cpp
    kotlinx/coroutines/common/src/internal/CoroutineExceptionHandlerImpl.common.cpp
    kotlinx/coroutines/common/src/internal/DispatchedContinuation.cpp
    kotlinx/coroutines/common/src/internal/DispatchedTask.cpp
    kotlinx/coroutines/common/src/internal/InlineList.cpp
    kotlinx/coroutines/common/src/internal/InternalAnnotations.common.cpp
    kotlinx/coroutines/common/src/internal/LimitedDispatcher.cpp
    kotlinx/coroutines/common/src/internal/LocalAtomics.common.cpp
    kotlinx/coroutines/common/src/internal/LockFreeLinkedList.common.cpp
    kotlinx/coroutines/common/src/internal/LockFreeTaskQueue.cpp
    kotlinx/coroutines/common/src/internal/MainDispatcherFactory.cpp
    kotlinx/coroutines/common/src/internal/NamedDispatcher.cpp
    kotlinx/coroutines/common/src/internal/OnUndeliveredElement.cpp
    kotlinx/coroutines/common/src/internal/ProbesSupport.common.cpp
    kotlinx/coroutines/common/src/internal/Scopes.cpp
    kotlinx/coroutines/common/src/internal/StackTraceRecovery.common.cpp
    kotlinx/coroutines/common/src/internal/Symbol.cpp
    kotlinx/coroutines/common/src/internal/Synchronized.common.cpp
    kotlinx/coroutines/common/src/internal/SystemProps.common.cpp
    kotlinx/coroutines/common/src/internal/ThreadContext.common.cpp
    kotlinx/coroutines/common/src/internal/ThreadLocal.common.cpp
    kotlinx/coroutines/common/src/internal/ThreadSafeHeap.cpp
    kotlinx/coroutines/common/src/intrinsics/Cancellable.cpp
    kotlinx/coroutines/common/src/intrinsics/Undispatched.cpp
    kotlinx/coroutines/common/src/selects/OnTimeout.cpp
    kotlinx/coroutines/common/src/selects/Select.cpp
    kotlinx/coroutines/common/src/selects/SelectOld.cpp
    kotlinx/coroutines/common/src/selects/SelectUnbiased.cpp
    kotlinx/coroutines/common/src/selects/WhileSelect.cpp
    kotlinx/coroutines/common/src/sync/Mutex.cpp
    kotlinx/coroutines/common/src/sync/Semaphore.cpp
        kotlinx/coroutines/concurrent/Builders.concurrent.cpp
        kotlinx/coroutines/concurrent/Dispatchers.cpp
        kotlinx/coroutines/concurrent/MultithreadedDispatchers.common.cpp
        kotlinx/coroutines/concurrent/channels/Channels.cpp
        kotlinx/coroutines/concurrent/internal/LockFreeLinkedList.cpp
        kotlinx/coroutines/concurrent/internal/OnDemandAllocatingPool.cpp
        kotlinx/coroutines/native/src/Builders.cpp
        kotlinx/coroutines/native/src/CloseableCoroutineDispatcher.cpp
        kotlinx/coroutines/native/src/CoroutineContext.cpp
        kotlinx/coroutines/native/src/Debug.cpp
        kotlinx/coroutines/native/src/Dispatchers.cpp
        kotlinx/coroutines/native/src/EventLoop.cpp
        kotlinx/coroutines/native/src/Exceptions.cpp
        kotlinx/coroutines/native/src/MultithreadedDispatchers.cpp
        kotlinx/coroutines/native/src/Runnable.cpp
        kotlinx/coroutines/native/src/SchedulerTask.cpp
        kotlinx/coroutines/native/src/flow/internal/FlowExceptions.cpp
        kotlinx/coroutines/native/src/flow/internal/SafeCollector.cpp
        kotlinx/coroutines/native/src/internal/Concurrent.cpp
        kotlinx/coroutines/native/src/internal/CopyOnWriteList.cpp
        kotlinx/coroutines/native/src/internal/CoroutineExceptionHandlerImpl.cpp
        kotlinx/coroutines/native/src/internal/LocalAtomics.cpp
        kotlinx/coroutines/native/src/internal/ProbesSupport.cpp
        kotlinx/coroutines/native/src/internal/StackTraceRecovery.cpp
        kotlinx/coroutines/native/src/internal/Synchronized.cpp
        kotlinx/coroutines/native/src/internal/SystemProps.cpp
        kotlinx/coroutines/native/src/internal/ThreadContext.cpp
        kotlinx/coroutines/native/src/internal/ThreadLocal.cpp
        kotlinx/coroutines/native/src/Dispatchers.cpp
)

# Create the core library
add_library(kotlinx-coroutines-core ${CORE_SOURCES})

# Set include directories for this target
target_include_directories(kotlinx-coroutines-core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        kotlinx/coroutines/common/src
        kotlinx/coroutines/common/src/channels
        kotlinx/coroutines/common/src/sync
        kotlinx/coroutines/common/src/flow
        kotlinx/coroutines/common/src/internal
        kotlinx/coroutines/common/src/selects
        kotlinx/coroutines/native/src
        kotlinx/coroutines/concurrent/src
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