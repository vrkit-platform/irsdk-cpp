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


#define IRSDK_CPP_TUPLE_PROP(name, index) \
  auto& name() {\
    return std::get<index>(*this);\
  }


// // BOOST_PP_COMMA_IF(BOOST_PP_LIST_IS_CONS(state))
// #define IRSDK_CPP_FOR_ELEM_ONLY(r, data, elem) BOOST_PP_TUPLE_ELEM(2,0,elem),

// #define IRSDK_CPP_HAS_NEXT(r, state) BOOST_PP_LIST_IS_CONS(state)
// #define IRSDK_CPP_NEXT(r, state) BOOST_PP_LIST_REST(state)
// #define IRSDK_CPP_TUPLE_STRUCT_TYPES(r, state) BOOST_PP_LIST_FOR_EACH_R(r, IRSDK_CPP_FOR_ELEM_ONLY, _, state)

// // Props example (int lap)(double lapTime)  
// #define IRSDK_CPP_TUPLE_STRUCT(Name, Props) \
//   struct Name : std::tuple<\
//     BOOST_PP_FOR(Props, IRSDK_CPP_HAS_NEXT, IRSDK_CPP_NEXT, IRSDK_CPP_TUPLE_STRUCT_TYPES)\
//     > {\
//     };


// // #define IRSDK_CPP_TUPLE_STRUCT_TEST
// IRSDK_CPP_TUPLE_STRUCT(TestStruct, (int lap)(double lapTime))