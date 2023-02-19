#include <d2d1_1.h>
#include <string>
#include <wincodec.h>

#include "CustomTextRenderer.h"
#include "define.h"
#include "Error.h"


class WICWrapper {
private:
    SaoFU::Param* param;

    ID2D1Factory** pD2DFactory;
    ID2D1RenderTarget** ppRenderTarget;

    IWICImagingFactory* pWICFactory;
    IWICBitmapDecoder* pWICDecoder;
    IWICBitmapFrameDecode* pWICFrameDecoder;
    IWICFormatConverter* pWICConverter;
    ID2D1Bitmap** ppBitmap;
    IWICBitmap* pWICBitmap;

    IWICStream* pStream;
    IWICBitmapEncoder* pEncoder;
    IWICBitmapFrameEncode* pFrameEncode;

    SAOFU_TRY_INIT();

public:

    WICWrapper(ID2D1Factory** pD2DFactory, ID2D1RenderTarget** pRenderTarget, SaoFU::Param *param)
    :
    pD2DFactory(pD2DFactory), ppRenderTarget(pRenderTarget), param(param) {
        D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, pD2DFactory);
    }

    void Release() {
        SafeRelease(pWICFactory);
        SafeRelease(pWICDecoder);
        SafeRelease(pWICFrameDecoder);
        SafeRelease(pWICConverter);
        SafeRelease(pWICBitmap);

        SafeRelease(*ppBitmap);
        SafeRelease(*ppRenderTarget);
        SafeRelease(*pD2DFactory);

        SafeRelease(pStream);
        SafeRelease(pEncoder);
        SafeRelease(pFrameEncode);

        CoUninitialize();
    }

    ~WICWrapper() {
        Release();
    }


    void CreateImageDecoder(ID2D1Bitmap** ppBitmap) {

        TRY_(CoInitialize(nullptr));

        TRY_
        (CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pWICFactory)));

        TRY_
        (pWICFactory->CreateDecoderFromFilename(param->path.c_str(), NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &pWICDecoder));

        TRY_(pWICDecoder->GetFrame(0, &pWICFrameDecoder));

        TRY_(pWICFactory->CreateFormatConverter(&pWICConverter));

        TRY_(
        pWICConverter->Initialize(
            pWICFrameDecoder,
            GUID_WICPixelFormat32bppPBGRA,
            WICBitmapDitherTypeNone,
            nullptr
            , 0.0,
            WICBitmapPaletteTypeCustom
        ));

        TRY_(pWICConverter->GetSize(&param->width, &param->height));

        if (param->max_width == -1) {
            param->max_width = param->width;
        }

        if (param->max_height == -1) {
            param->max_height = param->height;
        }

        TRY_
        (pWICFactory->CreateBitmap(param->width, param->height, GUID_WICPixelFormat32bppPBGRA, WICBitmapCacheOnLoad, &pWICBitmap));

        TRY_(
        (*pD2DFactory)->CreateWicBitmapRenderTarget(//CreateWicBitmapRenderTarget
            pWICBitmap,
            D2D1::RenderTargetProperties(),
            ppRenderTarget
        ));

        TRY_(
        (*ppRenderTarget)->CreateBitmapFromWicBitmap(
            pWICConverter,
            NULL,
            ppBitmap
        ));

        this->ppBitmap = ppBitmap;
    }

    void SaveImage() {
        TRY_(pWICFactory->CreateStream(&pStream));
        TRY_(pStream->InitializeFromFilename(param->name.c_str(), GENERIC_WRITE));

        TRY_(pWICFactory->CreateEncoder(param->container_format, nullptr, &pEncoder));
        TRY_(pEncoder->Initialize(pStream, WICBitmapEncoderNoCache));

        TRY_(pEncoder->CreateNewFrame(&pFrameEncode, nullptr));
        TRY_(pFrameEncode->Initialize(nullptr));

        TRY_(pFrameEncode->WriteSource(pWICBitmap, nullptr));

        TRY_(pFrameEncode->Commit());
        TRY_(pEncoder->Commit());

        this->pEncoder = pEncoder;
        this->pFrameEncode = pFrameEncode;
    }

};

int draw() {
    SaoFU::Param param;
    param.max_width = -1;
    param.max_height = -1;
    param.font_size = 500;
    param.text = L"洪老闆大蛋餅+1🪱";
    param.font_family = L"蘋方-繁";
    param.pos_x = 0;
    param.pos_y = 0;
    param.blur_level = 50;
    param.stroke_width = 50;
    param.path = L"C:\\Users\\Ussr\\OneDrive\\桌面\\圖片\\25_avatar_middle.jpg";
    param.name = L"output.png";
    param.in_solid_color_brush = D2D1::ColorF(D2D1::ColorF::White);
    param.out_solid_color_brush = D2D1::ColorF(0, 122, 204);
    param.container_format = GUID_ContainerFormatJpeg;

    ID2D1Factory* pD2DFactory = nullptr;
    ID2D1RenderTarget* pRenderTarget = nullptr;

    WICWrapper d2d(&pD2DFactory, &pRenderTarget, &param);

    ID2D1Bitmap* pBitmap = nullptr;
    d2d.CreateImageDecoder(&pBitmap);

    IDWriteFactory* pDWriteFactory = nullptr;
    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&pDWriteFactory));

    IDWriteTextFormat* pTextFormat = NULL;
    pDWriteFactory->CreateTextFormat(
        param.font_family.c_str(),
        NULL,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        param.font_size,
        L"zh-TW",
        &pTextFormat
    );

    IDWriteTextLayout* pTextLayout = NULL;
    pDWriteFactory->CreateTextLayout(
        param.text.c_str(),
        param.text.length(),
        pTextFormat,
        param.max_width,
        param.max_height,
        &pTextLayout
    );

    ID2D1SolidColorBrush* pSolidColorBrush = NULL;
    ID2D1SolidColorBrush* pSolidColorBrushOut = NULL;

    pRenderTarget->CreateSolidColorBrush(param.in_solid_color_brush, &pSolidColorBrush);
    pRenderTarget->CreateSolidColorBrush(param.out_solid_color_brush, &pSolidColorBrushOut);

    IDWriteTextRenderer* pTextRender = new CustomTextRenderer(pD2DFactory, pRenderTarget, pSolidColorBrush, pSolidColorBrushOut, param.stroke_width, param.blur_level);

    {
        pRenderTarget->BeginDraw();
        pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
        pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White, 0.0f));
        pRenderTarget->DrawBitmap(pBitmap);
        pTextLayout->Draw(NULL, pTextRender, param.pos_x, param.pos_y);
        pRenderTarget->EndDraw();
    }

    d2d.SaveImage();

    SafeRelease(pTextFormat);
    SafeRelease(pDWriteFactory);
    SafeRelease(pSolidColorBrush);
    SafeRelease(pTextRender);
    SafeRelease(pSolidColorBrushOut);
    SafeRelease(pTextLayout);

    return 0;
}

int main() {

    try {
        draw();
    }
    catch (SaoFU::Error& e) {
        MessageBoxA(0, e.what(), 0, MB_ICONERROR);
    }
   
    return 0;
}