#include <atomic>
#include <iostream>

#include <nan.h>

#include "stacks.h"
#include "stringify.h"
#include "sys.h"

/// static internals; set in init and read elsewhere
static uv_timer_t loop_watcher_timer = {};
static v8::Isolate *init_isolate = nullptr;
static std::atomic_bool already_initialised(false);

/// config; written in init and read from elsewhere
static uint64_t check_loop_every_ms = 100;
static uint64_t report_after_block_time_ms = 1000;

/// shared between the timer and the worker thread
static std::atomic_uint64_t loop_last_alive_ms(0);
/// has loop_last_alive_ms been reset since we last triggered an interrupt
static std::atomic_bool observed_this_block(false);
/// are we waiting for an interrupt to run (i.e. did we already trigger it)
static std::atomic_bool disable_calling_interrupt(false);

uint64_t block_estimate() { return wall_clock_time_ms() - loop_last_alive_ms; }

void interrupt_main(v8::Isolate *isolate, void *_data) {
  const uint64_t loop_blocked_ms = block_estimate();
  const std::string stack = current_stack_trace(isolate);

  std::ostringstream out;

  out << R"({"name":"dump-stacks","blockedMs":)" << loop_blocked_ms;
  out << R"(,"stack":")" << escape_json_string(stack) << "\"";
  out << "}";

  std::cerr << out.str() << std::endl;

  disable_calling_interrupt = false;
}

[[noreturn]] void *worker_thread_main(void *unused) {
  for (;;) {
    uv_sleep(check_loop_every_ms);

    if (disable_calling_interrupt || observed_this_block) {
      continue;
    }

    uint64_t loop_blocked_ms = wall_clock_time_ms() - loop_last_alive_ms;
    if (loop_blocked_ms < report_after_block_time_ms) {
      continue;
    }

    disable_calling_interrupt = true;
    observed_this_block = true;

    init_isolate->RequestInterrupt(interrupt_main, nullptr);
  }
}

void record_loop_times(uv_timer_t *timer) {
  loop_last_alive_ms = uv_now(timer->loop);
  observed_this_block = false;
}

void Init(v8::Local<v8::Object> exports, v8::Local<v8::Value> _module,
          void *_priv) {
  if (already_initialised) {
    Nan::ThrowError("this module cannot be loaded twice in a process");
    return;
  }
  already_initialised = true;

  const uint64_t observe_loop_timing_ms =
      getenv_u64_or("DUMP_STACKS_OBSERVE_MS", 100);
  check_loop_every_ms = getenv_u64_or("DUMP_STACKS_CHECK_MS", 100);
  report_after_block_time_ms =
      getenv_u64_or("DUMP_STACKS_REPORT_ONCE_MS", 1000);

  init_isolate = v8::Isolate::GetCurrent();

  if (0 != uv_timer_init(Nan::GetCurrentEventLoop(), &loop_watcher_timer)) {
    return Nan::ThrowError("creating timer");
  }
  if (0 != uv_timer_start(&loop_watcher_timer, record_loop_times,
                          observe_loop_timing_ms, observe_loop_timing_ms)) {
    return Nan::ThrowError("starting timer");
  }

  loop_last_alive_ms = uv_now(loop_watcher_timer.loop);

  // prevent the timer from interfering with process shutdown
  uv_unref(reinterpret_cast<uv_handle_t *>(&loop_watcher_timer));

  if (!create_thread(worker_thread_main)) {
    return;
  }

  v8::Local<v8::Context> context = exports->CreationContext();
  exports->Set(context, Nan::New("ready").ToLocalChecked(), Nan::New(true))
      .Check();
}

NODE_MODULE(dump_stacks, Init)
