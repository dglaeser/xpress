// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.a.glaeser@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once

#define ADD_2(x) x + x
#define ADD_4(x) ADD_2(x) + ADD_2(x)
#define ADD_8(x) ADD_4(x) + ADD_4(x)
#define ADD_16(x) ADD_8(x) + ADD_8(x)
#define ADD_32(x) ADD_16(x) + ADD_16(x)
#define ADD_64(x) ADD_32(x) + ADD_32(x)
#define ADD_192(x) ADD_64(x) + ADD_64(x) + ADD_64(x)

#define UNIT_EXPRESSION(a, b) a*((a + b)*b + (a*b) + b)
#define GENERATE_EXPRESSION(a, b) ADD_192(UNIT_EXPRESSION(a, b))
