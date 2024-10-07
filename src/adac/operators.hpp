// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Operators
 * \brief Arithmetic operators on symbols/expressions.
 */
#pragma once

#include <type_traits>
#include <functional>

#include <adac/utils.hpp>
#include <adac/dtype.hpp>
#include <adac/expressions.hpp>


namespace adac {

//! \addtogroup Operators
//! \{

namespace operators {

struct add : std::plus<void> {};
struct subtract : std::minus<void> {};
struct multiply : std::multiplies<void> {};
struct divide : std::divides<void> {};


namespace traits {

template<typename op> struct is_commutative : std::false_type {};
template<> struct is_commutative<add> : std::true_type {};
template<> struct is_commutative<multiply> : std::true_type {};

}  // namespace traits


template<typename op>
inline constexpr bool is_commutative_v = traits::is_commutative<op>::value;

}  // namespace operators


//! Class to represent expressions resulting from an operator applied to the given terms
template<typename op, concepts::expression... Ts>
struct expression : bindable<traits::common_dtype_t<traits::dtype_of_t<Ts>...>>, negatable {
    using bindable<traits::common_dtype_t<traits::dtype_of_t<Ts>...>>::operator=;

    constexpr expression() = default;
    constexpr expression(const op&, const Ts&...) noexcept {}
};

template<typename op, typename... Ts>
expression(op&&, Ts&&...) -> expression<std::remove_cvref_t<op>, std::remove_cvref_t<Ts>...>;


template<concepts::expression A, concepts::expression B>
    requires(!traits::disable_generic_arithmetic_operators<A, B>::value)
inline constexpr auto operator+(const A&, const B&) noexcept {
    return expression<operators::add, A, B>{};
}

template<concepts::expression A, concepts::expression B>
    requires(!traits::disable_generic_arithmetic_operators<A, B>::value)
inline constexpr auto operator-(const A&, const B&) noexcept {
    return expression<operators::subtract, A, B>{};
}

template<concepts::expression A, concepts::expression B>
    requires(!traits::disable_generic_arithmetic_operators<A, B>::value)
inline constexpr auto operator*(const A&, const B&) noexcept {
    return expression<operators::multiply, A, B>{};
}

template<concepts::expression A, concepts::expression B>
    requires(!traits::disable_generic_arithmetic_operators<A, B>::value)
inline constexpr auto operator/(const A&, const B&) noexcept {
    return expression<operators::divide, A, B>{};
}


namespace traits {

template<typename op, typename... Ts>
struct dtype_of<expression<op, Ts...>> : std::type_identity<typename expression<op, Ts...>::dtype> {};

template<typename op, typename T1, typename T2>
    requires(operators::is_commutative_v<op>)
struct is_equal_node<expression<op, T1, T2>, expression<op, T2, T1>> : std::true_type {};

template<typename op, typename T, typename... Ts>
struct nodes_of<expression<op, T, Ts...>> {
    using type = merged_types_t<
        merged_types_t<type_list<expression<op, T, Ts...>>, typename nodes_of<T>::type>,
        typename nodes_of<Ts>::type...
    >;
};

template<typename op, typename... Ts>
struct value_of<expression<op, Ts...>> {
    template<typename... V>
    static constexpr auto from(const bindings<V...>& binders) noexcept {
        using self = expression<op, Ts...>;
        if constexpr (bindings<V...>::template has_bindings_for<self>)
            return binders[self{}];
        else
            return op{}(evaluate(Ts{}, binders)...);
    }
};


// traits for addition
template<typename T1, typename T2>
struct derivative_of<expression<operators::add, T1, T2>> {
    template<typename V>
    static constexpr auto wrt(const type_list<V>& vars) noexcept {
        return differentiate(T1{}, vars) + differentiate(T2{}, vars);
    }
};

template<typename T1, typename T2>
struct stream<expression<operators::add, T1, T2>> {
    template<typename... V>
    static constexpr void to(std::ostream& out, const bindings<V...>& values) noexcept {
        write_to(out, T1{}, values);
        out << " + ";
        write_to(out, T2{}, values);
    }
};


// traits for subtraction
template<typename T1, typename T2>
struct derivative_of<expression<operators::subtract, T1, T2>> {
    template<typename V>
    static constexpr auto wrt(const type_list<V>& vars) noexcept {
        return differentiate(T1{}, vars) - differentiate(T2{}, vars);
    }
};

template<typename T1, typename T2>
struct stream<expression<operators::subtract, T1, T2>> {
    template<typename... V>
    static constexpr void to(std::ostream& out, const bindings<V...>& values) noexcept {
        write_to(out, T1{}, values);
        out << " - ";
        write_to(out, T2{}, values);
    }
};


// traits for multiplication
template<typename T1, typename T2>
struct derivative_of<expression<operators::multiply, T1, T2>> {
    template<typename V>
    static constexpr auto wrt(const type_list<V>& vars) noexcept {
        return differentiate(T1{}, vars)*T2{} + T1{}*differentiate(T2{}, vars);
    }
};

template<typename T1, typename T2>
struct stream<expression<operators::multiply, T1, T2>> {
    template<typename... V>
    static constexpr void to(std::ostream& out, const bindings<V...>& values) noexcept {
        static constexpr bool has_subterms_1 = type_list_size_v<nodes_of_t<T1>> > 1;
        if constexpr (has_subterms_1) out << "(";
        write_to(out, T1{}, values);
        if constexpr (has_subterms_1) out << ")";

        out << "*";

        static constexpr bool has_subterms_2 = type_list_size_v<nodes_of_t<T2>> > 1;
        if constexpr (has_subterms_2) out << "(";
        write_to(out, T2{}, values);
        if constexpr (has_subterms_2) out << ")";
    }
};


// traits for division
template<typename T1, typename T2>
struct derivative_of<expression<operators::divide, T1, T2>> {
    template<typename V>
    static constexpr auto wrt(const type_list<V>& vars) noexcept {
        return differentiate(T1{}, vars)/T2{} - T1{}*differentiate(T2{}, vars)/(T2{}*T2{});
    }
};

template<typename T1, typename T2>
struct stream<expression<operators::divide, T1, T2>> {
    template<typename... V>
    static constexpr void to(std::ostream& out, const bindings<V...>& values) noexcept {
        static constexpr bool has_subterms_1 = type_list_size_v<nodes_of_t<T1>> > 1;
        if constexpr (has_subterms_1) out << "(";
        write_to(out, T1{}, values);
        if constexpr (has_subterms_1) out << ")";

        out << "/";

        static constexpr bool has_subterms_2 = type_list_size_v<nodes_of_t<T2>> > 1;
        if constexpr (has_subterms_2) out << "(";
        write_to(out, T2{}, values);
        if constexpr (has_subterms_2) out << ")";
    }
};

}  // namespace traits

//! \} group Operators

}  // namespace adac
