#/* ==========================================================================
 Copyright (c) 2019 Andreas Pokorny
 Distributed under the Boost Software License, Version 1.0. (See accompanying
 file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
========================================================================== */

#pragma once

#include <version>

#if 0  // defined(__cpp_nontype_template_parameter_class) && defined(__cpp_impl_three_way_comparison)
#define HSM_USE_PROPER_LITERALS 1
#define HSM_TEMPLATE_LITERAL(x) template <str_lit x>
#else
// #define HSM_USE_PROPER_LITERALS 0
#define HSM_TEMPLATE_LITERAL(x) template <typename x>
#endif
