#include "shared.h"

#ifndef STOCK_API

#ifdef BUILDING_IW1X_DLL
#define STOCK_API __declspec(dllexport)
#else
#define STOCK_API __declspec(dllimport)
#endif

#endif

namespace stock
{}

namespace cvars
{}