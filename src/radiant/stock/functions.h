#ifndef STOCK_FUNCTIONS_H
#define STOCK_FUNCTIONS_H


#ifndef STOCK_API

#ifdef BUILDING_IW1X_DLL
#define STOCK_API __declspec(dllexport)
#else
#define STOCK_API __declspec(dllimport)
#endif

#endif

#include "stock/constants.h"

namespace stock
{}

#endif
