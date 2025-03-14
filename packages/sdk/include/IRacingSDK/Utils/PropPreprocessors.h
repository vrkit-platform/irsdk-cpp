#pragma once

#include <tuple>

#include <boost/preprocessor/for.hpp>
#include <boost/preprocessor/comma.hpp>
#include <boost/preprocessor/comma_if.hpp>
#include <boost/preprocessor/list/adt.hpp>
#include <boost/preprocessor/list/for_each.hpp>
#include <boost/preprocessor/repetition/for.hpp>
#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/preprocessor/comparison/not_equal.hpp>

#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/seq/elem.hpp>


#define IRCPP_TUPLE_PROP(name, index) \
  auto& name() {\
    return std::get<index>(*this);\
  }


// // BOOST_PP_COMMA_IF(BOOST_PP_LIST_IS_CONS(state))
// #define IRCPP_FOR_ELEM_ONLY(r, data, elem) BOOST_PP_TUPLE_ELEM(2,0,elem),

// #define IRCPP_HAS_NEXT(r, state) BOOST_PP_LIST_IS_CONS(state)
// #define IRCPP_NEXT(r, state) BOOST_PP_LIST_REST(state)
// #define IRCPP_TUPLE_STRUCT_TYPES(r, state) BOOST_PP_LIST_FOR_EACH_R(r, IRCPP_FOR_ELEM_ONLY, _, state)

// // Props example (int lap)(double lapTime)  
// #define IRCPP_TUPLE_STRUCT(Name, Props) \
//   struct Name : std::tuple<\
//     BOOST_PP_FOR(Props, IRCPP_HAS_NEXT, IRCPP_NEXT, IRCPP_TUPLE_STRUCT_TYPES)\
//     > {\
//     };


// // #define IRCPP_TUPLE_STRUCT_TEST
// IRCPP_TUPLE_STRUCT(TestStruct, (int lap)(double lapTime))