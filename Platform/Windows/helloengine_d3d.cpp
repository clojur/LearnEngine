#include<windows.h>
#include<windowsx.h>
#include<tchar.h>
#include<stdint.h>

#include<d3d11.h>
#include<d3d11_1.h>
#include<d3dcompiler.h>
#include<DirectXMath.h>
#include<DirectXPackedVector.h>
#include<DirectXColors.h>

using namespace DirectX;
using namespace DirectX::PackedVector;

const uint32_t SCREEN_WIDTH=960;
const uint32_t SCREEN_HEIGHT=480;

IDXGISwapChain        *g_pSwapchain=nullptr;
ID3D11Device          *g_pDev=nullptr;
ID3D11DeviceContext   *g_pDevcon=nullptr;

ID3D11RenderTargetView *g_pRTView=nullptr;

ID3D11InputLayout*      g_pLayout=nullptr;
ID3D11VertexShader*     g_pVS=nullptr;
ID3D11PixelShader*      g_pPS=nullptr;

ID3D11Buffer*           g_pVBuffer=nullptr;

struct VERTEX
{
  XMFLOAT3 Position;
  XMFLOAT4 Color;
};

template<class T>
inline void SafeRelease(T **ppInterfaceToRelease)
{
  if(*ppInterfaceToRelease!=nullptr)
    {
      (*ppInterfaceToRelease)->Release();
      (*ppInterfaceToRelease)=nullptr;
    }
}

void CreateRenderTarget()
{
  HRESULT hr;
  ID3D11Texture2D* pBackBuffer;
  g_pSwapchain->GetBuffer(0,__uuidof(ID3D11Texture2D),(LPVOID*)&pBackBuffer);

  g_pDev->CreateRenderTargetView(pBackBuffer,NULL,&g_pRTView);

  pBackBuffer->Release();

  g_pDevcon->OMSetRenderTargets(1,&g_pRTView,NULL);
}

void SetViewport()
{
  D3D11_VIEWPORT viewport;
  ZeroMemory(&viewport,sizeof(D3D11_VIEWPORT));

  viewport.TopLeftX=0;
  viewport.TopLeftY=0;
  viewport.Width=SCREEN_WIDTH;
  viewport.Height=SCREEN_HEIGHT;

  g_pDevcon->RSSetViewports(1,&viewport);
}

void InitPipeline()
{
  ID3DBlob *VS, *PS;
  D3DReadFileToBlob(L"copy.vso",&VS);
  D3DReadFileToBlob(L"copy.pso",&PS);

  g_pDev->CreateVertexShader(VS->GetBufferPointer(),VS->GetBufferSize(),NULL,&g_pVS);
  g_pDev->CreatePixelShader(PS->GetBufferPointer(),PS->GetBufferSize(),NULL,&g_pPS);

  g_pDevcon->VSSetShader(g_pVS,0,0);
  g_pDevcon->PSSetShader(g_pPS,0,0);

  D3D11_INPUT_ELEMENT_DESC  ied[]=
  {
      {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
      {"COLOR",0,DXGI_FORMAT_R32G32B32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0}
  };

  g_pDev->CreateInputLayout(ied,2,VS->GetBufferPointer(),VS->GetBufferSize(),&g_pLayout);
  g_pDevcon->IAGetInputLayout(g_pLayout);

  VS->Release();
  PS->Release();
}

void InitGrapics()
{
  VERTEX OurVertices[]=
  {
    {XMFLOAT3(0.0f,0.5f,0.0f),XMFLOAT4(1.0f,0.0f,0.0f,1.0f)},
    {XMFLOAT3(0.45f,-0.5f,0.0f),XMFLOAT4(0.0f,1.0f,0.0f,1.0f)},
    {XMFLOAT3(-0.45f,-0.5f,0.0f),XMFLOAT4(0.0f,0.0f,1.0f,1.0f)}
  };

  D3D11_BUFFER_DESC bd;

  bd.Usage=D3D11_USAGE_DYNAMIC;
  bd.ByteWidth=sizeof(VERTEX)*3;
  bd.BindFlags=D3D11_BIND_VERTEX_BUFFER;
  bd.CPUAccessFlags=D3D11_CPU_ACCESS_WRITE;

  g_pDev->CreateBuffer(&bd,NULL,&g_pVBuffer);

  D3D11_MAPPED_SUBRESOURCE ms;
  g_pDevcon->Map(g_pVBuffer,NULL,D3D11_MAP_WRITE_DISCARD,NULL,&ms);
  memcpy(ms.pData,OurVertices,sizeof(VERTEX)*3);
  g_pDevcon->Unmap(g_pVBuffer,NULL);
};


HRESULT CreateGraphicsResources(HWND hWnd)
{
  HRESULT hr=S_OK;
  if(pRenderTarget==nullptr)
    {
      RECT rc;
         GetClientRect(hWnd,&rc);
      
      D2D1_SIZE_U size=D2D1::SizeU(rc.right-rc.left,rc.bottom-rc.top);
      hr=pFactory->CreateHwndRenderTarget(
					  D2D1::RenderTargetProperties(),
					  D2D1::HwndRenderTargetProperties(hWnd,size),
					  &pRenderTarget
       				  );
      if(SUCCEEDED(hr))
	{
	  hr=pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LightSlateGray),&pLightSlateGrayBrush);
	}
      if(SUCCEEDED(hr))
	{
	  hr=pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::CornflowerBlue),&pCornflowerBlueBrush);
	}
      
      return hr;
    }

  return hr;
 
}

void DiscardGraphicsResources()
{
  SafeRelease(&pRenderTarget);
  SafeRelease(&pLightSlateGrayBrush);
  SafeRelease(&pCornflowerBlueBrush);
}



LRESULT CALLBACK WindowProc(HWND hWnd,
			    UINT message,
			    WPARAM wParam,
			    LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance,
		   HINSTANCE hPrevInstance,
		   LPTSTR lpCmdLine,
		   int nCmdShow)
{
  HWND hWnd;
  
  WNDCLASSEX wc;

  if(FAILED(CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED|COINIT_DISABLE_OLE1DDE)))  return -1;


  ZeroMemory(&wc,sizeof(WNDCLASSEX));

  wc.cbSize=sizeof(WNDCLASSEX);
  wc.style=CS_HREDRAW|CS_VREDRAW;
  wc.lpfnWndProc=WindowProc;
  wc.hInstance=hInstance;
  wc.hCursor=LoadCursor(NULL,IDC_ARROW);
  wc.hbrBackground=(HBRUSH)COLOR_WINDOW;
  wc.lpszClassName=_T("WindowClass1");

  RegisterClassEx(&wc);

  hWnd=CreateWindowEx(0,
		      _T("Windowclass1"),
		      _T("Hello,Engine![Dirct 2D]"),
		      WS_OVERLAPPEDWINDOW,
		      100,
		      100,
		      960,
		      540,
		      NULL,
		      NULL,
		      hInstance,
		      NULL);

  ShowWindow(hWnd,nCmdShow);

  MSG msg;

  while(GetMessage(&msg,NULL,0,0))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  
  CoUninitialize();
  return msg.wParam;
  
}


LRESULT CALLBACK WindowProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
  LRESULT result=0;
  bool wasHandled=false;
  switch(message)
  {
  case WM_CREATE:
    {
      if(FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,&pFactory)))
	{
	  result=-1;
	  return result;
	}
      wasHandled=true;
      result=0;
      break;
    }
  case WM_PAINT:
    {
      HRESULT hr=CreateGraphicsResources(hWnd);
      if(SUCCEEDED(hr))
	{
	  PAINTSTRUCT ps;
	  BeginPaint(hWnd,&ps);

	  pRenderTarget->BeginDraw();
	  pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
	  D2D_SIZE_F rtSize=pRenderTarget->GetSize();

	  int width=static_cast<int>(rtSize.width);
	  int height=static_cast<int>(rtSize.height);
	  
	  for(int x=0;x<width;x+=10)
	    {
	      pRenderTarget->DrawLine(
				      D2D1::Point2F(static_cast<FLOAT>(x),0.0f),
				      D2D1::Point2F(static_cast<FLOAT>(x),rtSize.height),
				      pLightSlateGrayBrush,
				      0.5f
				      );
	    }
	  
	  for(int y=0;y<height;y+=10)
	    {
	      pRenderTarget->DrawLine(
				      D2D1::Point2F(0.0f,static_cast<FLOAT>(y)),
				      D2D1::Point2F(rtSize.width,static_cast<FLOAT>(y)),
				      pLightSlateGrayBrush,
				      0.5f
				   );
	    }

	  D2D1_RECT_F rectangle1=D2D1::RectF(
					     rtSize.width/2-50.0f,
					     rtSize.height/2-50.0f,
					     rtSize.width/2+50.0f,
					     rtSize.height/2+50.0f
					     );

	  D2D1_RECT_F rectangle2=D2D1::RectF(
					     rtSize.width/2-100.0f,
					     rtSize.height/2-100.0f,
					     rtSize.width/2+100.0f,
					     rtSize.height/2+100.0f
					     );

	  pRenderTarget->FillRectangle(&rectangle1,pLightSlateGrayBrush);
	  
	  pRenderTarget->DrawRectangle(&rectangle2,pCornflowerBlueBrush);
	  
	  hr=pRenderTarget->EndDraw();
	  if(FAILED(hr)||hr==D2DERR_RECREATE_TARGET)
	    {
	      DiscardGraphicsResources();
	    }

	  EndPaint(hWnd,&ps);

	}
      wasHandled=true;
      break;
    }
    
  case WM_SIZE:
    {
      if(pRenderTarget!=nullptr)
	{
	  RECT rc;
	  GetClientRect(hWnd,&rc);
	  D2D1_SIZE_U size=D2D1::SizeU(rc.right-rc.left,rc.bottom-rc.top);
	  
	  pRenderTarget->Resize(size);
	}
      wasHandled=true;
      break;
    } 
  case WM_DESTROY:
    {
      DiscardGraphicsResources();
      if(pFactory){pFactory->Release();pFactory=nullptr;}
      PostQuitMessage(0);
      result=0;
      wasHandled=true;
      return 0;
    }break;
  case WM_DISPLAYCHANGE:
    {
      InvalidateRect(hWnd,nullptr,false);
      wasHandled=true;
      break;
    }
  }

  if(!wasHandled)  result=  DefWindowProc(hWnd,message,wParam,lParam);
  return result;
}
