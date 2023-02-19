#include <Windows.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <wincodec.h>
#include <dwrite.h>
#include <shlwapi.h>
#include <string>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "dwrite.lib")
#include "CustomTextRenderer.h"
#include "define.h"

HRESULT CustomTextRenderer::DrawGlyphRun(
    void* clientDrawingContext, FLOAT baselineOriginX, FLOAT baselineOriginY,
    DWRITE_MEASURING_MODE measuringMode, DWRITE_GLYPH_RUN const* glyphRun,
    DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription, IUnknown* clientDrawingEffect)
{
    HRESULT hr = S_OK;

    ID2D1PathGeometry* pPathGeometry = nullptr;
    hr = m_pD2DFactory->CreatePathGeometry(&pPathGeometry);
    ID2D1GeometrySink* pSink = nullptr;
    hr = pPathGeometry->Open(&pSink);

    hr = glyphRun->fontFace->GetGlyphRunOutline(
        glyphRun->fontEmSize,
        glyphRun->glyphIndices,
        glyphRun->glyphAdvances,
        glyphRun->glyphOffsets,
        glyphRun->glyphCount,
        glyphRun->isSideways,
        glyphRun->bidiLevel,
        pSink
    );
    hr = pSink->Close();

    D2D1::Matrix3x2F const matrix = D2D1::Matrix3x2F(
        1.0f, 0.0f,
        0.0f, 1.0f,
        baselineOriginX, baselineOriginY
    );
    

    ID2D1TransformedGeometry* pTransformedGeometry = nullptr;
    hr = m_pD2DFactory->CreateTransformedGeometry(pPathGeometry, &matrix, &pTransformedGeometry);

    ID2D1BitmapRenderTarget* pCompatibleRenderTarget = NULL;

    m_pRenderTarget->CreateCompatibleRenderTarget(
        m_pRenderTarget->GetSize(),
        &pCompatibleRenderTarget
    );

    ID2D1DeviceContext* d2dDeviceContext;
    pCompatibleRenderTarget->QueryInterface(&d2dDeviceContext);

    ID2D1Effect* pGaussianBlurEffect = nullptr;
    d2dDeviceContext->CreateEffect(CLSID_D2D1GaussianBlur, &pGaussianBlurEffect);
    pGaussianBlurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, mBlurLevel);

    {
        pCompatibleRenderTarget->BeginDraw();
        pCompatibleRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White, 0));
        // 绘制文字描边部分
        pCompatibleRenderTarget->DrawGeometry(pTransformedGeometry, m_pTextOutlineBrush, mStrokeWidth);
        // 绘制文字填充部分
        pCompatibleRenderTarget->EndDraw();
    }

    ID2D1Bitmap* pGridBitmap = NULL;
    pCompatibleRenderTarget->GetBitmap(&pGridBitmap);

    pGaussianBlurEffect->SetInput(0, pGridBitmap);

    ID2D1DeviceContext* pRenderTargetContext;
    m_pRenderTarget->QueryInterface(&pRenderTargetContext);

    {
        pRenderTargetContext->DrawImage(pGaussianBlurEffect);
        m_pRenderTarget->FillGeometry(pTransformedGeometry, m_pTextBodyBrush);
    }

    SafeRelease(pRenderTargetContext);
    SafeRelease(d2dDeviceContext);
    SafeRelease(pGaussianBlurEffect);

    SafeRelease(pGridBitmap);
    SafeRelease(pCompatibleRenderTarget);

    SafeRelease(pPathGeometry);
    SafeRelease(pSink);
    SafeRelease(pTransformedGeometry);

    return hr;
}
