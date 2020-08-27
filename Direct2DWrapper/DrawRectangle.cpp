#include "pch.h"
#include "DrawRectangle.h"

namespace Direct2DWrapper
{

	template <class T> void SafeRelease(T** ppT)
	{
		if (*ppT)
		{
			(*ppT)->Release();
			*ppT = NULL;
		}
	}

	DIRECT2DWRAPPER_C_FUNCTION
		double Add(int a, double b)
	{
		return a + b;
	}

	DIRECT2DWRAPPER_C_FUNCTION
		ID2D1Factory* CreateD2D1Factory()
	{
		ID2D1Factory* pD2D1Factory = NULL;
		HRESULT hr = D2D1CreateFactory(
			D2D1_FACTORY_TYPE_SINGLE_THREADED,
			&pD2D1Factory
		);
		return pD2D1Factory;
	}

	DIRECT2DWRAPPER_C_FUNCTION
		void ReleaseD2D1Factory(ID2D1Factory* pD2D1Factory)
	{
		SafeRelease(&pD2D1Factory);
	}

	DIRECT2DWRAPPER_C_FUNCTION
		IWICImagingFactory* CreateImagingFactory()
	{
		IWICImagingFactory* pWICImagingFactory = NULL;

		HRESULT hr = CoCreateInstance(
			CLSID_WICImagingFactory,
			nullptr,
			CLSCTX_INPROC_SERVER,
			__uuidof(IWICImagingFactory),
			reinterpret_cast<void**>(&pWICImagingFactory)
		);
		return pWICImagingFactory;
	}

	DIRECT2DWRAPPER_C_FUNCTION
		void ReleaseImagingFactory(IWICImagingFactory* pWICImagingFactory)
	{
		SafeRelease(&pWICImagingFactory);
	}

	DIRECT2DWRAPPER_C_FUNCTION
		IWICBitmap* CreateWICBitmap(IWICImagingFactory* pWICImagingFactory, UINT width, UINT height)
	{
		IWICBitmap* pWICBitmap = NULL;
		HRESULT hr = pWICImagingFactory->CreateBitmap(
			width,
			height,
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapCacheOnLoad,
			&pWICBitmap
		);
		return pWICBitmap;
	}

	DIRECT2DWRAPPER_C_FUNCTION
		void ReleaseWICBitmap(IWICBitmap* pWICBitmap)
	{
		SafeRelease(&pWICBitmap);
	}

	DIRECT2DWRAPPER_C_FUNCTION
		ID2D1RenderTarget* CreateRenderTarget(ID2D1Factory* pD2D1Factory, IWICBitmap* pWICBitmap)
	{
		D2D1_RENDER_TARGET_PROPERTIES targetProperties = D2D1::RenderTargetProperties();
		D2D1_PIXEL_FORMAT desiredFormat = D2D1::PixelFormat(
			DXGI_FORMAT_B8G8R8A8_UNORM,
			D2D1_ALPHA_MODE_PREMULTIPLIED
		);
		targetProperties.pixelFormat = desiredFormat;

		ID2D1RenderTarget* pD2D1RenderTarget = NULL;
		HRESULT hr = pD2D1Factory->CreateWicBitmapRenderTarget(
			pWICBitmap,
			targetProperties,
			&pD2D1RenderTarget
		);
		return pD2D1RenderTarget;
	}

	DIRECT2DWRAPPER_C_FUNCTION
		void ReleaseRenderTarget(ID2D1RenderTarget* pD2D1RenderTarget)
	{
		SafeRelease(&pD2D1RenderTarget);
	}

	DIRECT2DWRAPPER_C_FUNCTION
		bool DrawImage(ID2D1RenderTarget* pD2D1RenderTarget)
	{
		pD2D1RenderTarget->BeginDraw();
		pD2D1RenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

		HRESULT hr = pD2D1RenderTarget->EndDraw();
		if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
		{
			SafeRelease(&pD2D1RenderTarget);
			return false;
		}
		return true;
	}

	DIRECT2DWRAPPER_C_FUNCTION
		HRESULT SaveImage(IWICImagingFactory* pWICImagingFactory, IWICBitmap* pWICBitmap, ID2D1RenderTarget* pD2D1RenderTarget, PCWSTR filename)
	{

		IWICStream* pWICStream = NULL;
		IWICBitmapEncoder* pWICBitmapEncoder = NULL;
		IWICBitmapFrameEncode* pWICBitmapFrameEncode = NULL;
		WICPixelFormatGUID pixelFormat = GUID_WICPixelFormatDontCare;

		HRESULT hr = pWICImagingFactory->CreateStream(&pWICStream);

		if (SUCCEEDED(hr))
		{
			hr = pWICStream->InitializeFromFilename(
				filename,
				GENERIC_WRITE
			);
		}
		if (SUCCEEDED(hr))
		{
			hr = pWICImagingFactory->CreateEncoder(
				GUID_ContainerFormatPng,
				NULL,
				&pWICBitmapEncoder
			);
		}
		if (SUCCEEDED(hr))
		{
			hr = pWICBitmapEncoder->Initialize(
				pWICStream,
				WICBitmapEncoderNoCache
			);
		}
		if (SUCCEEDED(hr))
		{
			hr = pWICBitmapEncoder->CreateNewFrame(
				&pWICBitmapFrameEncode,
				NULL
			);
		}
		if (SUCCEEDED(hr))
		{
			hr = pWICBitmapFrameEncode->Initialize(NULL);
		}
		if (SUCCEEDED(hr))
		{
			D2D1_SIZE_U renderTargetSize = pD2D1RenderTarget->GetPixelSize();
			hr = pWICBitmapFrameEncode->SetSize(
				renderTargetSize.width,
				renderTargetSize.height
			);
		}
		if (SUCCEEDED(hr))
		{
			hr = pWICBitmapFrameEncode->SetPixelFormat(&pixelFormat);
		}
		if (SUCCEEDED(hr))
		{
			hr = pWICBitmapFrameEncode->WriteSource(pWICBitmap, NULL);
		}
		if (SUCCEEDED(hr))
		{
			hr = pWICBitmapFrameEncode->Commit();
		}
		if (SUCCEEDED(hr))
		{
			hr = pWICBitmapEncoder->Commit();
		}

		SafeRelease(&pWICBitmapFrameEncode);
		SafeRelease(&pWICBitmapEncoder);
		SafeRelease(&pWICStream);

		return hr;
	}

}
