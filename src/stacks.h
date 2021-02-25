#include <ostream>
#include <sstream>
#include <string>

#include <nan.h>

/// Write out a string which looks something like a node stack trace line.
///
/// e.g. make_potato (/srv/app/node_modules/potatoes/potato.js:43:7)
/// e.g. [eval] (12:1)
/// e.g. internal.js:17:1
///
/// no escaping is done, i.e. may be able to produce multiple lines or nulls
///
/// c.f.
/// https://github.com/nodejs/node/blob/63794b419814e6b7ae106b6c90752a59bef6ff3e/deps/v8/src/objects/stack-frame-info.cc#L356
/// c.f.
/// https://github.com/nodejs/node-report/blob/b19df9b4f8f3fce58c7fa2408b2bf2060781515e/src/module.cc#L220
static void write_stack_frame(std::ostream &out,
                              v8::Local<v8::StackFrame> frame) {
  const Nan::Utf8String func_name(frame->GetFunctionName());

  bool had_func_name = false;
  if (frame->IsEval()) {
    out << "[eval]";
    had_func_name = true;
  } else if (func_name.length() != 0) {
    out << *func_name;
    had_func_name = true;
  }

  if (had_func_name) {
    out << " (";
  }

  const Nan::Utf8String script_name(frame->GetScriptName());
  if (script_name.length() != 0) {
    out << *script_name << ":";
  }

  out << frame->GetLineNumber() << ":" << frame->GetColumn();

  if (had_func_name) {
    out << ")";
  }

  out << "\n";
}

static void write_current_stack_trace(std::ostream &out, v8::Isolate *isolate) {
  v8::Local<v8::StackTrace> stack = v8::StackTrace::CurrentStackTrace(
      isolate, 255, v8::StackTrace::kDetailed);
  if (stack.IsEmpty()) {
    return;
  }

  for (int i = 0; i < stack->GetFrameCount(); i++) {
    write_stack_frame(out, stack->GetFrame(isolate, i));
  }
}

static std::string current_stack_trace(v8::Isolate *isolate) {
  std::ostringstream ss;
  write_current_stack_trace(ss, isolate);
  return ss.str();
}
