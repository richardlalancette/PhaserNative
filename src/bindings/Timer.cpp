#include "Timer.h"

JSC_FUNCTION(Timer::createTimer) {
    JSC::Value callbackVal = JSC::Value(ctx, argv[0]);
    JSC::Value delayVal    = JSC::Value(ctx, argv[1]);
    JSC::Value oneshotVal  = JSC::Value(ctx, argv[2]);

    unsigned int delay = delayVal.toInteger();
    bool oneshot = oneshotVal.toBoolean();

    if (delay < 1) { delay = 1; }

    SDL_Event *event = new SDL_Event();
    SDL_memset(event, 0, sizeof(SDL_Event));
    event->type = PhaserNativeEvent::Timeout;
    event->user.code = oneshot ? 1 : 0;

    PhaserNativeEvent::Timers.push_back(event);

    event->user.data1 = (void*)((JSValueRef)callbackVal);
    event->user.data2 = reinterpret_cast<void*>(
                SDL_AddTimer(delay, +[](Uint32 interval, void *param) -> Uint32 {
                    SDL_Event *event = reinterpret_cast<SDL_Event *>(param);
                    SDL_PushEvent(event);
                    if (event->user.code) { // if oneshot
                                            return 0; // timer will self-destruct.
                    } else {
                        return interval; // repeat this timer.
                    }
                }, event));

    return JSC::Value::MakeUndefined(ctx);
}

JSC_FUNCTION(Timer::deleteTimer) {
    JSC::Value timerIDVal = JSC::Value(ctx, argv[0]);
    int timer_id = timerIDVal.toInteger();

    if (SDL_RemoveTimer(timer_id)) {
        auto it = PhaserNativeEvent::Timers.begin();
        auto end = PhaserNativeEvent::Timers.end();
        while (it != end) {
            SDL_Event *timeoutEvent = *it;
            int other_timer_id = static_cast<int>(reinterpret_cast<intptr_t>(timeoutEvent->user.data2));
            if (timer_id == other_timer_id) {
                delete timeoutEvent;
                PhaserNativeEvent::Timers.erase(it);
                break;
            }
            ++it;
        }
    }
    return JSC::Value::MakeUndefined(ctx);
}

void Timer::Register(JSContextRef ctx)
{
    JSC::String createTimerFunctionName = JSC::String(ctx, "__createTimer");
    JSC::String deleteTimerFunctionName = JSC::String(ctx, "__deleteTimer");

    JSObjectRef createTimerFunctionObject = JSObjectMakeFunctionWithCallback(ctx, createTimerFunctionName, createTimer);
    JSObjectRef deleteTimerFunctionObject = JSObjectMakeFunctionWithCallback(ctx, deleteTimerFunctionName, deleteTimer);

    JSObjectSetProperty(ctx, JSContextGetGlobalObject(ctx), createTimerFunctionName, createTimerFunctionObject, kJSPropertyAttributeDontEnum | kJSPropertyAttributeDontDelete, nullptr);
    JSObjectSetProperty(ctx, JSContextGetGlobalObject(ctx), deleteTimerFunctionName, deleteTimerFunctionObject, kJSPropertyAttributeDontEnum | kJSPropertyAttributeDontDelete, nullptr);

    JSC::evaluateScriptFromString(ctx,
                "function setTimeout(func, delay) {\n"
                "    var cb_func;\n"
                "    var bind_args;\n"
                "    var timer_id;\n"
                "    if (typeof delay !== 'number') {\n"
                "        if (typeof delay === 'undefined') {\n"
                "            delay = 0;\n"
                "        } else {\n"
                "            throw new TypeError('invalid delay');\n"
                "        }\n"
                "    }\n"
                "    if (typeof func === 'string') {\n"
                "        cb_func = eval.bind(this, func);\n"
                "    } else if (typeof func !== 'function') {\n"
                "        throw new TypeError('callback is not a function/string');\n"
                "    } else if (arguments.length > 2) {\n"
                "        bind_args = Array.prototype.slice.call(arguments, 2);\n"
                "        bind_args.unshift(this);\n"
                "        cb_func = func.bind.apply(func, bind_args);\n"
                "    } else {\n"
                "        cb_func = func;\n"
                "    }\n"
                "    timer_id = __createTimer(cb_func, delay, true);\n"
                "    return timer_id;\n"
                "}\n"

                "function clearTimeout(timer_id) {\n"
                "    if (typeof timer_id !== 'number') {\n"
                "        throw new TypeError('timer ID is not a number');\n"
                "    }\n"
                "    var success = __deleteTimer(timer_id);\n"
                "}\n"

                "function setInterval(func, delay) {\n"
                "    var cb_func;\n"
                "    var bind_args;\n"
                "    var timer_id;\n"
                "    if (typeof delay !== 'number') {\n"
                "        if (typeof delay === 'undefined') {\n"
                "            delay = 0;\n"
                "        } else {\n"
                "            throw new TypeError('invalid delay');\n"
                "        }\n"
                "    }\n"
                "    if (typeof func === 'string') {\n"
                "        cb_func = eval.bind(this, func);\n"
                "    } else if (typeof func !== 'function') {\n"
                "        throw new TypeError('callback is not a function/string');\n"
                "    } else if (arguments.length > 2) {\n"
                "        bind_args = Array.prototype.slice.call(arguments, 2);\n"
                "        bind_args.unshift(this);\n"
                "        cb_func = func.bind.apply(func, bind_args);\n"
                "    } else {\n"
                "        cb_func = func;\n"
                "    }\n"
                "    timer_id = __createTimer(cb_func, delay, false);\n"
                "    return timer_id;\n"
                "}\n"

                "function clearInterval(timer_id) {\n"
                "    if (typeof timer_id !== 'number') {\n"
                "        throw new TypeError('timer ID is not a number');\n"
                "    }\n"
                "    __deleteTimer(timer_id);\n"
                "}\n",

                "bindings/Timer.cpp");
}