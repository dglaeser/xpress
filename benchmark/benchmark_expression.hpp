#pragma once

#define ADD_2(x) x + x
#define ADD_4(x) ADD_2(x) + ADD_2(x)
#define ADD_8(x) ADD_4(x) + ADD_4(x)
#define ADD_16(x) ADD_8(x) + ADD_8(x)
#define ADD_32(x) ADD_16(x) + ADD_16(x)
#define ADD_64(x) ADD_32(x) + ADD_32(x)
#define ADD_128(x) ADD_64(x) + ADD_64(x)

#define UNIT_EXPRESSION(a, b) val<2>*a*((a + b)*b*val<4> + (a*b) + b*val<8>)
#define GENERATE_EXPRESSION(a, b) ADD_128(UNIT_EXPRESSION(a, b))
