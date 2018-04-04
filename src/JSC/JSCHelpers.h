#pragma once

#include "JSCContext.h"
#include "JSCClass.h"
#include "JSCException.h"

#include <vector>

#define JSC_GLOBAL_CTX JSC::GlobalContext::GetInstance()
#define JSC_GLOBAL_OBJECT JSC::Object::getGlobalObject()

#define JSC_CONSTRUCTOR(_CONSTRUCTOR_NAME_) \
    JSObjectRef _CONSTRUCTOR_NAME_(JSContextRef ctx, JSObjectRef object, size_t argc, const JSValueRef argv[], JSValueRef* exception)

#define JSC_INITIALIZER(_INITIALIZER_NAME_) \
    void _INITIALIZER_NAME_(JSContextRef ctx, JSObjectRef object)

#define JSC_FINALIZER(_FINALIZER_NAME_) \
    void _FINALIZER_NAME_(JSObjectRef object)

#define JSC_FUNCTION(_FUNC_NAME_) \
    JSValueRef _FUNC_NAME_(JSContextRef ctx, JSObjectRef function, JSObjectRef object, size_t argc, const JSValueRef argv[], JSValueRef* exception)

#define JSC_PROPERTY_GET(_GETTER_NAME_) \
    JSValueRef _GETTER_NAME_(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception)

#define JSC_PROPERTY_SET(_SETTER_NAME_) \
    bool _SETTER_NAME_(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef value, JSValueRef* exception)


#define JSC_CONSTANT(_CONST_NAME_) \
    [](JSContextRef, JSObjectRef, JSStringRef, JSValueRef*) -> JSValueRef { return JSC::Value(_CONST_NAME_); }, nullptr, kJSPropertyAttributeReadOnly

namespace JSC
{

template<class C>
class Binding {
public:

    static JSC::Object CreateObject()
    {
        // this is how we force subclasses to write their GetClassRef
        // implementation. If you get an error on this line, you forgot to
        // implement 'static JSC::Class &GetClassRef()' in the subclass.
        return JSC::Object::Make(C::GetClassRef());
    }

    operator JSObjectRef() const {
        return object;
    }

    operator JSValueRef() const {
        return object;
    }

    static size_t GetNextIndex()
    {
        size_t index = 1; // 0 is reserved.
        bool found = false;
        for (index = 1; index < _pool.size(); ++index) {
            if (!_pool[index]._inUse) {
                found = true;
                break;
            }
        }
        if (!found) {
            _pool.resize(index+1);
        }
        _pool[index]._inUse = true;
        return index;
    }

    static C &CreateInstance(JSC::Object object) {
        size_t index = GetNextIndex();
        C &instance = _pool[index];
        instance.object = std::move(object);
        instance.object.setPrivate(index);
        return instance;
    }

    static void FreeInstance(JSC::Object object) {
        size_t index = object.getPrivate<size_t>();
        _pool[index]._inUse = false;
    }

    static C &GetInstance(size_t index) {
        return _pool[index];
    }

    static C &GetInstance(JSC::Object object) {
        return _pool[object.getPrivate<size_t>()];
    }

    static size_t GetInstanceCount() {
        size_t count = 0;
        for (size_t index = 1; index < _pool.size(); ++index) {
            if (_pool[index]._inUse) { ++count; }
        }
        return count;
    }

    JSC::Object object;

protected:

    static Class _class;


private:
    bool _inUse = false;
    static std::vector<C> _pool;

};

template<class C> Class Binding<C>::_class;
template<class C> std::vector<C> JSC::Binding<C>::_pool;


Value evaluateScriptFromFile(const char *path);
Value evaluateScriptFromFileHandler(FILE *f, const char *sourceURL);
Value evaluateScriptFromString(const std::string &scriptUTF8, const char *sourceURL);
Value evaluateScript(const String &scriptUTF8, const String &sourceURL);

}
