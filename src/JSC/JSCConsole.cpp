#include <cstdarg>

#include <SDL2/SDL_log.h>

#include "JSCHelpers.h"

JSC_BINDINGS(JSCConsole)
{
    static JSStaticFunction staticFunctions[] = {
        { "log", [](JSContextRef ctx, JSObjectRef /*function*/, JSObjectRef /*thiz*/, size_t argc, const JSValueRef argv[], JSValueRef* exception) -> JSValueRef {
              try {
                  std::string output;
                  for(size_t i = 0; i < argc; i++) {
                      if (i > 0){ output += " "; }
                      output += JSC::Value(ctx, argv[i]).toString().getUTF8String();
                  }
                  SDL_Log("%s\n", output.c_str());
              } catch(const std::exception & e) {
                  JSC::Value message(ctx, JSC::String(ctx, e.what()));
                  JSValueRef args[] { message, nullptr };
                  *exception = JSObjectMakeError(ctx, 1, args, nullptr);
              }
              return JSValueMakeUndefined(ctx);
          }, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
        { "info", [](JSContextRef ctx, JSObjectRef /*function*/, JSObjectRef /*thiz*/, size_t argc, const JSValueRef argv[], JSValueRef* exception) -> JSValueRef {
              try {
                  std::string output;
                  for(size_t i = 0; i < argc; i++) {
                      if (i > 0){ output += " "; }
                      output += JSC::Value(ctx, argv[i]).toString().getUTF8String();
                  }
                  SDL_LogInfo(0, "%s\n", output.c_str());
              } catch(const std::exception & e) {
                  JSC::Value message(ctx, JSC::String(ctx, e.what()));
                  JSValueRef args[] { message, nullptr };
                  *exception = JSObjectMakeError(ctx, 1, args, nullptr);
              }
              return JSValueMakeUndefined(ctx);
          }, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
        { "warn", [](JSContextRef ctx, JSObjectRef /*function*/, JSObjectRef /*thiz*/, size_t argc, const JSValueRef argv[], JSValueRef* exception) -> JSValueRef {
              try {
                  std::string output;
                  for(size_t i = 0; i < argc; i++) {
                      if (i > 0){ output += " "; }
                      output += JSC::Value(ctx, argv[i]).toString().getUTF8String();
                  }
                  SDL_LogWarn(0, "%s\n", output.c_str());
              } catch(const std::exception & e) {
                  JSC::Value message(ctx, JSC::String(ctx, e.what()));
                  JSValueRef args[] { message, nullptr };
                  *exception = JSObjectMakeError(ctx, 1, args, nullptr);
              }
              return JSValueMakeUndefined(ctx);
          }, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
        { "error", [](JSContextRef ctx, JSObjectRef /*function*/, JSObjectRef /*thiz*/, size_t argc, const JSValueRef argv[], JSValueRef* exception) -> JSValueRef {
              try {
                  std::string output;
                  for(size_t i = 0; i < argc; i++) {
                      if (i > 0){ output += " "; }
                      output += JSC::Value(ctx, argv[i]).toString().getUTF8String();
                  }
                  SDL_LogError(0, "%s\n", output.c_str());
              } catch(const std::exception & e) {
                  JSC::Value message(ctx, JSC::String(ctx, e.what()));
                  JSValueRef args[] { message, nullptr };
                  *exception = JSObjectMakeError(ctx, 1, args, nullptr);
              }
              return JSValueMakeUndefined(ctx);
          }, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
        { 0, 0, 0 }
    };

    JSClassDefinition classDefinition = kJSClassDefinitionEmpty;
    classDefinition.className = "console";
    classDefinition.attributes = kJSClassAttributeNoAutomaticPrototype;
    classDefinition.staticFunctions = staticFunctions;
    JSClassRef console_class = JSClassCreate(&classDefinition);

    auto ctx = JSC::globalContext();
    JSObjectSetProperty(ctx,
                        JSContextGetGlobalObject(ctx),
                        JSStringCreateWithUTF8CString("console"),
                        JSObjectMake(ctx, console_class, nullptr),
                        kJSPropertyAttributeNone, nullptr);

}
