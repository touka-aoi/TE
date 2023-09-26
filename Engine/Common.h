#include <d3d12.h>
#include "Types.h"
#include "Log.h"
#include <cassert>

#define KILOBYTE 1024ull
#define MEGABYTE (1024ull*KILOBYTE)
#define GIGABYTE (1024ull*MEGABYTE)
#define TERABYTE (1024ull*GIGABYTE)

template<class T>
T AlignOffset(const T& uOffset, const T& uAlign) { return ((uOffset + (uAlign - 1)) & ~(uAlign - 1)); }
// (uOffset + (uAlign - 1)) アライメントに一番近い位置に先頭を移動させる、-1することで一番近い位置に移動できる ex : 6 + ( 4 - 1 ) = 9 ( 8 が一番近い ) この誤差は次の右項で調整する
// ~(uAlign - 1) ビット反転したアライメントに&演算すると先頭ビット以外0になりアライメントが整う

template<class... Args>
void SetName(ID3D12Object* pObj, const char* format, Args&& ... args)
{
    char bufName[240];
    sprintf_s(bufName, format, args...);
    std::string Name = bufName;
    std::wstring wName(Name.begin(), Name.end());
    pObj->SetName(wName.c_str());
}


inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        assert(false);// throw HrException(hr);
    }
}
