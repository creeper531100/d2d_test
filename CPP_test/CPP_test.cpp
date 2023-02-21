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

        HR_T(CoInitialize(nullptr));

        HR_T
        (CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pWICFactory)));

        HR_T
        (pWICFactory->CreateDecoderFromFilename(param->path.c_str(), NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &pWICDecoder));

        HR_T(pWICDecoder->GetFrame(0, &pWICFrameDecoder));

        HR_T(pWICFactory->CreateFormatConverter(&pWICConverter));

        HR_T(
        pWICConverter->Initialize(
            pWICFrameDecoder,
            GUID_WICPixelFormat32bppPBGRA,
            WICBitmapDitherTypeNone,
            nullptr
            , 0.0,
            WICBitmapPaletteTypeCustom
        ));

        HR_T(pWICConverter->GetSize(&param->width, &param->height));

        if (param->max_width == -1) {
            param->max_width = param->width;
        }

        if (param->max_height == -1) {
            param->max_height = param->height;
        }

        HR_T
        (pWICFactory->CreateBitmap(param->width, param->height, GUID_WICPixelFormat32bppPBGRA, WICBitmapCacheOnLoad, &pWICBitmap));

        HR_T(
        (*pD2DFactory)->CreateWicBitmapRenderTarget(//CreateWicBitmapRenderTarget
            pWICBitmap,
            D2D1::RenderTargetProperties(),
            ppRenderTarget
        ));

        HR_T(
        (*ppRenderTarget)->CreateBitmapFromWicBitmap(
            pWICConverter,
            NULL,
            ppBitmap
        ));

        this->ppBitmap = ppBitmap;
    }

    void SaveImage() {
        HR_T(pWICFactory->CreateStream(&pStream));
        HR_T(pStream->InitializeFromFilename(param->name.c_str(), GENERIC_WRITE));

        HR_T(pWICFactory->CreateEncoder(param->container_format, nullptr, &pEncoder));
        HR_T(pEncoder->Initialize(pStream, WICBitmapEncoderNoCache));

        HR_T(pEncoder->CreateNewFrame(&pFrameEncode, nullptr));
        HR_T(pFrameEncode->Initialize(nullptr));

        HR_T(pFrameEncode->WriteSource(pWICBitmap, nullptr));

        HR_T(pFrameEncode->Commit());
        HR_T(pEncoder->Commit());

        this->pEncoder = pEncoder;
        this->pFrameEncode = pFrameEncode;
    }

};

int draw() {
    SaoFU::Param param;
    param.max_width = -1;
    param.max_height = -1;
    param.font_size = 200;
    param.text = L"洪老闆大眼睛";
    param.font_family = L"蘋方-繁";
    
    param.blur_level = 50;
    param.stroke_width = 50;
    param.path = L"C:\\Users\\Ussr\\OneDrive\\桌面\\圖片\\329615121_6082902801772509_2484708322112285115_n.jpg";
    param.name = L"output.png";
    param.in_solid_color_brush = D2D1::ColorF(D2D1::ColorF::White);
    param.out_solid_color_brush = D2D1::ColorF(D2D1::ColorF::BurlyWood);
    param.container_format = GUID_ContainerFormatJpeg;

    ID2D1Factory* pD2DFactory = nullptr;
    ID2D1RenderTarget* pRenderTarget = nullptr;

    WICWrapper d2d(&pD2DFactory, &pRenderTarget, &param);

    ID2D1Bitmap* pBitmap = nullptr;
    d2d.CreateImageDecoder(&pBitmap);

    param.pos_x = (param.width / 2) - (param.font_size * param.text.length()) / 2;
    param.pos_y = (param.height / 2) - (param.font_size / 2);

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

#include <dxgi1_4.h>
#include <d3d12.h>

#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

int main() {
    try {
        draw();
    }
    catch (SaoFU::Error& e) {
        MessageBoxA(0, e.what(), 0, MB_ICONERROR);
    }
    
    return 0;
}


/*
try {
    D3D12_COMMAND_LIST_TYPE d3d_list_type = D3D12_COMMAND_LIST_TYPE_DIRECT;


    IDXGIFactory4* p_factory = NULL;
    HR_T(CreateDXGIFactory1(IID_PPV_ARGS(&p_factory)));

    ID3D12Device* p_device = NULL;
    HR_T(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&p_device)));

    D3D12_COMMAND_QUEUE_DESC queue_desc = {};
    {
        queue_desc.Type = d3d_list_type;
        queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    }
    ID3D12CommandQueue* p_queue = NULL;
    HR_T(p_device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&p_queue)));

    ID3D12CommandAllocator* p_cmd_alloc = NULL;
    HR_T
    (p_device->CreateCommandAllocator(d3d_list_type, IID_PPV_ARGS(&p_cmd_alloc)));

    ID3D12GraphicsCommandList* p_graphics_cmd_list = NULL;
    HR_T
    (p_device->CreateCommandList(0, d3d_list_type, p_cmd_alloc, NULL, IID_PPV_ARGS(&p_graphics_cmd_list)));

    ID3D12CommandList* cmd_list [] = {p_graphics_cmd_list};
    p_queue->ExecuteCommandLists(_countof(cmd_list), cmd_list);
    //p_graphics_cmd_list->Reset(p_cmd_alloc, );

    HR_T(p_cmd_alloc->Reset());
    HR_T(p_graphics_cmd_list->Close());

    SafeRelease(p_graphics_cmd_list);
    SafeRelease(p_cmd_alloc);
    SafeRelease(p_queue);
    SafeRelease(p_device);
    SafeRelease(p_factory);
}
catch (SaoFU::Error& e) {
    MessageBoxA(0, e.what(), 0, MB_ICONERROR);
}
*/
