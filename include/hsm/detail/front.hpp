/* ==========================================================================
 Copyright (c) 2019 Andreas Pokorny
 Distributed under the Boost Software License, Version 1.0. (See accompanying
 file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
========================================================================== */

#ifndef HSM_DETAIL_FRONT_HPP_INCLUDED
#define HSM_DETAIL_FRONT_HPP_INCLUDED

#include <utility>
#include "hsm/hsm_fwd.hpp"

namespace hsm
{
namespace detail
{
template <typename D>
struct is_state_ref : kvasir::mpl::bool_<false>
{
};
HSM_TEMPLATE_LITERAL(D)
struct is_state_ref<hsm::state_ref<D>> : kvasir::mpl::bool_<true>
{
};

template <typename D>
struct is_internal : kvasir::mpl::bool_<false>
{
};
template <>
struct is_internal<internal_transition> : kvasir::mpl::bool_<true>
{
};
template <typename D>
struct is_final_state : kvasir::mpl::bool_<false>
{
};
template <typename D>
struct is_final_state<final_state<D>> : kvasir::mpl::bool_<true>
{
};
}  // namespace detail
}  // namespace hsm

#endif
