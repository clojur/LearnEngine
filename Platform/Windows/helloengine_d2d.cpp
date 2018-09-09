#include<windows.h>
#include<windowsx.h>
#include<tchar.h>

#include<d2d1.h>

ID2D1Factory		*pFactory=nullptr;
ID2D1HwndRenderTarget	*pRenderTarget=nullptr;
ID2D1SolidColorBrush	*pLightSlateGrayBrush=nullptr; 
ID2D1SolidColorBrush	*pCornflowerBlueBrush=nullptr;

template<class T>
inline void SafeRelease(T **ppInterfaceToRelease)
{
  if(*ppInterfaceToRelease!=nullptr)
    {
      (*ppInterfaceToRelease)->Release();

      (*ppInterfaceToRelease)=nullptr;
    }
}

HRESULT CreateGraphicsResources(HWND hWnd)
{
  HRESULT hr=S_OK;
  if(pRenderTarget==nullptr)
    {
      RECT rc;
      GetClientRect(hWnd,&rc);

      D2D_SIZE_U size=D2D1::SizeU(rc.right-rc.left,rc.bottom-rc.top);

      hr=pFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(),
					  D2D1::HwndRenderTargetProperties(hWnd,size),
					  &pRenderTarget);

      if(SUCCEEDED(hr))
	{
	  hr==pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::CornflowerBule),&pCornflowerBlueBrush);
	  
	}
    }
  return hr;
}

void DiscardGFraphicsResources()
{
  SafeRelease(&pRenderTarget);
  SafeRelease(&pLightSlateGrayBrush);
  SafeRelease(&pCornflowerBlueBrush);
  
}

LRESULT CALLBACK WindowProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);

int WINAPI  WinMain(HINSTANCE hInstance,
		   HINSTANCE hPrevInstance,
		   LPTSTR lpCmdLine,
		   int nCmdShow)
{
  HWND hWnd;

  WNDCLASSEX wc;

  if(FAILED(CoInitializeEX(nullptr,CONINIT_APARTMENTTHREADED|CONINIT_DISABLE_OLE1DDE)))
    return -1;
  
  ZeroMemory(&wc,sizeof(WNDCLASSEX));

  wc.cbSize=sizeof(WNDCLASSEX);
  wc.style=CS_HREDRAW|CS_VREDRAW;
  wc.lpfnWndProc=WindowProc;
  wc.hInstance=hInstance;
  wc.hCursor=LoadCursor(nullptr,IDC_ARROW);
  wc.hbrBackground=(HBRUSH)COLOR_WINDOW;
  wc.lpszClassName=_T("WindowClass1");

  RegisterClassEx(&wc);

  hWnd=CreateWindowEx(0,
		      _T("Windowclass1"),
		      _T("Hello,Engine![Direct 2D]"),
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

  while(GetMessage(&msg,nullptr,0,0))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  CoUninitialize();

  return msg.wParam;
  
}


LRESULT CALLBACK WindowProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
  LRESULT result=0
    bool wasHandled=false;
  
  switch(message)
  {
  case WM_CREATE:
    {
      if(FAILED(D2D1CreateFactory(
				  D2D_FACTORY_TYPE_SINGLE_THREADED,&pFactory)))
	{
	  result=-1;
	  return result;
	}
      wasHandled=true;
      result=0;
    }break;
  case WM_PAINT:
  {
    PAINTSTRUCT ps;
    HDC hdc=BeginPaint(hWnd,&ps);
    RECT rec={20,20,60,80};
    HBRUSH brush=(HBRUSH)GetStockObject(BLACK_BRUSH);

    FillRect(hdc,&rec,brush);

    EndPaint(hWnd,&ps);
  }break;
  case WM_DESTROY:
    {
      PostQuitMessage(0);
      return 0;
    }break;
  }

  return DefWindowProc(hWnd,message,wParam,lParam);
}
