#pragma once

#include <d2d1.h>
#include <dwrite.h>

#define SafeRelease(x)              \
    if(x) {                         \
        (x)->Release();             \
        (x) = NULL;                 \
    } else {                        \
        MessageBoxA(0, 0, 0, 0);    \
    }


class CustomTextRenderer : public IDWriteTextRenderer {
protected:
    ULONG m_cRef;

    ID2D1Factory* m_pD2DFactory;
    ID2D1RenderTarget* m_pRenderTarget;
    ID2D1Brush* m_pTextBodyBrush;
    ID2D1SolidColorBrush* m_pTextOutlineBrush;
    float mStrokeWidth;
    float mBlurLevel;

public:
    CustomTextRenderer(
        ID2D1Factory* pD2DFactory, ID2D1RenderTarget* pRenderTarget,
        ID2D1Brush* pTextBodyBrush, ID2D1SolidColorBrush* pTextOutlineBrush,
        float strokeWidth = 1.0f,
        float BlurLevel = 1.0f)
        :
        m_pD2DFactory(pD2DFactory),
        m_pRenderTarget(pRenderTarget),
        m_pTextBodyBrush(pTextBodyBrush),
        m_pTextOutlineBrush(pTextOutlineBrush),
        mStrokeWidth(strokeWidth),
        mBlurLevel(BlurLevel)
    {}

    ~CustomTextRenderer() {
        MessageBox(0, L"Release", L"Release", 0);
    }

    STDMETHOD(DrawGlyphRun) (
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_MEASURING_MODE measuringMode,
        DWRITE_GLYPH_RUN const* glyphRun,
        DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
        IUnknown* clientDrawingEffect
    );

    STDMETHOD(DrawUnderline)(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_UNDERLINE const* underline,
        IUnknown* clientDrawingEffect
    ){
        return E_NOTIMPL;
    }

    STDMETHOD(DrawStrikethrough)(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_STRIKETHROUGH const* strikethrough,
        IUnknown* clientDrawingEffect
    ){
        return E_NOTIMPL;
    }

    STDMETHOD(DrawInlineObject)(
        void* clientDrawingContext,
        FLOAT originX,
        FLOAT originY,
        IDWriteInlineObject* inlineObject,
        BOOL isSideways,
        BOOL isRightToLeft,
        IUnknown* clientDrawingEffect
    ) {
        return E_NOTIMPL;
    }

    STDMETHOD(IsPixelSnappingDisabled)(
        void* clientDrawingContext,
        BOOL* isDisabled
    ) {
        *isDisabled = FALSE;
        return S_OK;
    }

    STDMETHOD(GetCurrentTransform)(
        void* clientDrawingContext,
        DWRITE_MATRIX* transform
    ) {
        m_pRenderTarget->GetTransform(reinterpret_cast<D2D1_MATRIX_3X2_F*>(transform));
        return S_OK;
    }

    STDMETHOD(GetPixelsPerDip)(
        void* clientDrawingContext,
        FLOAT* pixelsPerDip
    ) {
        float x, yUnused;
        m_pRenderTarget->GetDpi(&x, &yUnused);
        *pixelsPerDip = x / 96;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void FAR* FAR* ppvObj) {
        if (iid == IID_IUnknown /*|| iid == IID_IDWritePixelSnapping || iid == IID_IDWriteTextRenderer*/) {
            *ppvObj = this;
            AddRef();
            return NOERROR;
        }
        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef() {
        return ++m_cRef;
    }

    ULONG STDMETHODCALLTYPE Release() {
        // Decrement the object's internal counter.
        if (0 == --m_cRef) {
            delete this;
        }
        return m_cRef;
    }
};
