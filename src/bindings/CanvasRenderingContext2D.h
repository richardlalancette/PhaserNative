#pragma once

#include <JSC/JSCHelpers.h>

class CanvasRenderingContext2D : public JSC::Binding<CanvasRenderingContext2D>
{
public:

    static JSC::Class &GetClassRef();

    static JSC_CONSTRUCTOR(Constructor);
    static JSC_FINALIZER(Finalizer);

    static JSC_FUNCTION(fillRect);

    static JSC_FUNCTION(createImageData);
    static JSC_FUNCTION(getImageData);
    static JSC_FUNCTION(putImageData);
    static JSC_FUNCTION(drawImage);

    static JSC_FUNCTION(measureText);
    static JSC_FUNCTION(fillText);

    static JSC_FUNCTION(save);
    static JSC_FUNCTION(restore);

    static JSC_FUNCTION(translate);

    static JSC_PROPERTY_GET(getFillStyle);
    static JSC_PROPERTY_SET(setFillStyle);

private:

    JSC::Value m_fillStyle;

};
