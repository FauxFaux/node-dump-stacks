#include <cinttypes>
#include <cstdlib>
#include <pthread.h>

#include <nan.h>
#include <uv.h>

static uint64_t wall_clock_time_ms() { return uv_hrtime() / 1000 / 1000; }

static uint64_t event_loop_time_ms() { return uv_now(uv_default_loop()); }

static void or_throw_code(int code, const char *message) {
  if (!code) {
    return;
  }

  std::ostringstream ss;
  ss << message << " failed: " << code;
  throw std::domain_error(ss.str());
}

static void create_thread(void *(*thread_main)(void *unused)) {
  pthread_attr_t attr = {};
  or_throw_code(pthread_attr_init(&attr), "pthread attr init");

  // thin down the thread; optional
  pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

  sigset_t sigmask, previous;
  or_throw_code(sigfillset(&sigmask), "sigfillset");
  or_throw_code(pthread_sigmask(SIG_SETMASK, &sigmask, &previous), "sigmask");
  pthread_t thread;
  const int err = pthread_create(&thread, &attr, thread_main, nullptr);
  or_throw_code(pthread_sigmask(SIG_SETMASK, &previous, nullptr),
                "restore old sigmask");
  or_throw_code(pthread_attr_destroy(&attr), "pthread attr destroy");

  or_throw_code(err, "creating thread");
}

static std::string getenv_string(const char *name) {
  const char *found = std::getenv(name);
  if (nullptr == found) {
    return "";
  }
  return found;
}

static uint64_t getenv_u64_or(const char *name, uint64_t fallback) {
  const char *raw = std::getenv(name);
  if (!raw) {
    return fallback;
  }

  const uint64_t val = std::stoull(raw);

  if (0 == val) {
    std::ostringstream ss;
    ss << "environment variable '" << name << "' cannot parse as zero";
    throw std::domain_error(ss.str());
  }

  return val;
}
