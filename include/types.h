#pragma once

#define INT_TYPE   long long
#define FLOAT_TYPE long double

typedef INT_TYPE   int_t;
typedef FLOAT_TYPE flt_t;

#define TO_INT static_cast<int_t>
#define TO_FLT static_cast<flt_t>
