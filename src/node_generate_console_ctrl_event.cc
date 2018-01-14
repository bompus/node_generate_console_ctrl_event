#include <napi.h>
#include <windows.h>

Napi::Value Send(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 2) {
        Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
        return env.Null();
    }

    if (!info[0].IsNumber()) {
        Napi::TypeError::New(env, "Argument 0 must be a number").ThrowAsJavaScriptException();
        return env.Null();
    }

    if (!info[1].IsString()) {
        Napi::TypeError::New(env, "Argument 1 must be a string").ThrowAsJavaScriptException();
        return env.Null();
    }

    uint32_t pid = info[0].As<Napi::Number>().Uint32Value();
    std::string event = info[1].As<Napi::String>().Utf8Value();
    uint32_t sendEvent = 0;

    if (event == "CTRL_C_EVENT") { sendEvent = CTRL_C_EVENT; }
    else if (event == "CTRL_BREAK_EVENT") { sendEvent = CTRL_BREAK_EVENT; }
    else {
        Napi::TypeError::New(env, "Argument 1 is not CTRL_C_EVENT or CTRL_BREAK_EVENT").ThrowAsJavaScriptException();
        return env.Null();
    }

    // we can only be attached to one console at a time, so we free our current console
    FreeConsole();

    // attach to the console of specified PID
    bool attached = AttachConsole(pid);

	if (attached) {
        // disable our own CTRL_*_EVENT handling, since we have to send the event to our entire group
        SetConsoleCtrlHandler(NULL, true);

        // send the specified event
        GenerateConsoleCtrlEvent(sendEvent, 0);
        
        // we sleep for a bit so that our own process does not receive the event
        Sleep(10);

        // we can only be attached to one console at a time, so we free our current console
        FreeConsole();

        // re-enable our own CTRL_*_EVENT handling, or else our own process would still be ignoring it
        SetConsoleCtrlHandler(NULL, false);
    }

    // re-attach to our own console, or our output disappears
    AttachConsole(ATTACH_PARENT_PROCESS);

    return Napi::Boolean::New(env, attached);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "send"), Napi::Function::New(env, Send));
  return exports;
}

NODE_API_MODULE(addon, Init)
