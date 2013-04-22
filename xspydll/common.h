#pragma once
#include <string>

//see afxtrace.cpp 这里是一种根据宏如WIN64_XXXX有没有定义，而切换宏的方法，相当于宏里定义宏
// from http://stackoverflow.com/questions/2664703/macros-in-macros-c
#include <boost/preprocessor/control/if.hpp>
#define PP_CHECK_N(x, n, ...) n
#define PP_CHECK(...) PP_CHECK_N(__VA_ARGS__, 0,)
#ifdef _WIN64
#define WIN64_XXXX ~, 1,
#endif
#define FAKE_POINTER_FORMAT \
    BOOST_PP_IF(PP_CHECK(WIN64_XXXX), "%#p", "0x%08x")
// 对于正直指针类型%p输出才会带0，而LONG_PTR不是指针类型，输出的时候前面不会带0

#define BOOST_FORMAT boost::str(boost::format

std::string GetMods(LPCVOID addr);
std::string GetCodes(PVOID addr);