#include "Canvas.h"
#include "CanvasRenderingContext2D.h"
#include "WebGLRenderingContext.h"

JSC_INITIALIZER(Canvas::Initializer) {
    size_t index = _AllocateInstance();
    JSC::Object instance = object;
    instance.setPrivate(index);
    instance.setProperty("2d", CanvasRenderingContext2D::Create());
    instance.setProperty("webgl", WebGLRenderingContext::Create());
    instance.setProperty("experimental-webgl", WebGLRenderingContext::Create());
}

JSC_FINALIZER(Canvas::Finalizer) {
    size_t index = (size_t)JSObjectGetPrivate(object);
    _FreeInstance(index);
}

JSC_FUNCTION(Canvas::getContext) {
    JSC::String elementName = JSC::Value(argv[0]).toString();
    JSC::Object instance = object;
    return instance.getProperty(elementName);
}

JSC::Class &Canvas::GetClassRef()
{
    if (!_class)
    {
        static JSStaticFunction canvasStaticFunctions[] = {
            { "getContext", Canvas::getContext, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
            { 0, 0, 0 }
        };

        JSClassDefinition canvasClassDefinition = kJSClassDefinitionEmpty;
        canvasClassDefinition.className = "HTMLCanvasElement";
        canvasClassDefinition.attributes = kJSClassAttributeNone;
        canvasClassDefinition.staticFunctions = canvasStaticFunctions;
        canvasClassDefinition.initialize = Canvas::Initializer;
        canvasClassDefinition.finalize = Canvas::Finalizer;
        _class = JSC::Class(&canvasClassDefinition);

        JSC::GlobalContext &ctx = JSC::GlobalContext::GetInstance();
        JSC_GLOBAL_OBJECT.setProperty("HTMLCanvasElement", Create());
    }

    return _class;
}
