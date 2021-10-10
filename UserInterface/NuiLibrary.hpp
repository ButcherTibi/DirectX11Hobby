#pragma once

// DXGI
#pragma comment(lib, "DXGI.lib")

// DirectX 11
#pragma comment(lib, "D3D11.lib")

// @C++_BUG: Modules Bug
// error C2953: 'std::_Tree_simple_types': class template has already been defined
// 
// @C++_BUG_WORKAROUND:
// #include <map>


import NuiLibrary;
