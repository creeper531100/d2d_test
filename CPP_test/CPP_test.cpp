#include <d2d1_1.h>
#include <string>
#include <wincodec.h>

#include "CustomTextRenderer.h"
#include "Error.h"

struct Param {
    Param() {}

    UINT width;
    UINT height;
    UINT max_width;
    UINT max_height;

    float font_size;
    std::wstring text;
    std::wstring font_family;

    UINT pos_x;
    UINT pos_y;

    UINT blur_level;
    UINT stroke_width;

    std::wstring path;
    std::wstring name;
    GUID container_format;

    D2D1::ColorF in_solid_color_brush = 0;
    D2D1::ColorF out_solid_color_brush = 0;
};

class Direct2DWrapper {
private:
    Param* param;

    ID2D1Factory** pD2DFactory;
    ID2D1RenderTarget** pRenderTarget;

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

    void Release() {
        SafeRelease(*ppBitmap);
        SafeRelease(pWICFactory);
        SafeRelease(pWICDecoder);
        SafeRelease(pWICFrameDecoder);
        SafeRelease(pWICConverter);
        SafeRelease(pWICBitmap);
        SafeRelease(*pRenderTarget);

        SafeRelease(*pD2DFactory);
        SafeRelease(pStream);
        SafeRelease(pEncoder);
        SafeRelease(pFrameEncode);

        CoUninitialize();
    }

    Direct2DWrapper(ID2D1Factory** pD2DFactory, ID2D1RenderTarget** pRenderTarget, Param *param)
    :
    pD2DFactory(pD2DFactory), pRenderTarget(pRenderTarget), param(param) {
        D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, pD2DFactory);
    }

    void CreateImageDecoder(ID2D1Bitmap** ppBitmap) {

        CoInitialize(nullptr);

        CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pWICFactory));

        pWICFactory->CreateDecoderFromFilename(param->path.c_str(), NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &pWICDecoder);

        pWICDecoder->GetFrame(0, &pWICFrameDecoder);

        pWICFactory->CreateFormatConverter(&pWICConverter);

        pWICConverter->Initialize(
            pWICFrameDecoder,
            GUID_WICPixelFormat32bppPBGRA,
            WICBitmapDitherTypeNone,
            nullptr
            , 0.0,
            WICBitmapPaletteTypeCustom
        );

        pWICConverter->GetSize(&param->width, &param->height);

        if (param->max_width == -1) {
            param->max_width = param->width;
        }

        if (param->max_height == -1) {
            param->max_height = param->height;
        }

        pWICFactory->CreateBitmap(param->width, param->height, GUID_WICPixelFormat32bppPBGRA, WICBitmapCacheOnLoad, &pWICBitmap);
        (*pD2DFactory)->CreateWicBitmapRenderTarget(//CreateWicBitmapRenderTarget
            pWICBitmap,
            D2D1::RenderTargetProperties(),
            pRenderTarget
        );

        (*pRenderTarget)->CreateBitmapFromWicBitmap(
            pWICConverter,
            NULL,
            ppBitmap
        );

        this->ppBitmap = ppBitmap;
    }

    void SaveImage() {
        pWICFactory->CreateStream(&pStream);
        pStream->InitializeFromFilename(param->name.c_str(), GENERIC_WRITE);

        pWICFactory->CreateEncoder(param->container_format, nullptr, &pEncoder);
        pEncoder->Initialize(pStream, WICBitmapEncoderNoCache);

        pEncoder->CreateNewFrame(&pFrameEncode, nullptr);
        pFrameEncode->Initialize(nullptr);

        pFrameEncode->WriteSource(pWICBitmap, nullptr);

        pFrameEncode->Commit();
        pEncoder->Commit();

        this->pEncoder = pEncoder;
        this->pFrameEncode = pFrameEncode;
    }

    ~Direct2DWrapper() {
        Release();
    }
};

int draw() {
    Param param;
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

    Direct2DWrapper d2d(&pD2DFactory, &pRenderTarget, &param);

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