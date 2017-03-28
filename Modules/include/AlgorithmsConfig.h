
#ifndef ALGORITHMSCONFIG_H
#define ALGORITHMSCONFIG_H

//
// 编译器符号导出通用扩展，支持Visual C++,Sun ONE Studio 8和HP aC++.
//
#if ((defined(_MSC_VER) || defined(_WIN32_WCE)) && !defined(GISE_STATIC_LIBS)) || (defined(__HP_aCC) && defined(__HP_WINDLL))
#   define ALGORITHMS_DECLSPEC_EXPORT __declspec(dllexport)
#   define ALGORITHMS_DECLSPEC_IMPORT __declspec(dllimport)
#elif defined(__SUNPRO_CC) && (__SUNPRO_CC >= 0x550)
#   define ALGORITHMS_DECLSPEC_EXPORT __global
#   define ALGORITHMS_DECLSPEC_IMPORT
#else
#   define ALGORITHMS_DECLSPEC_EXPORT /**/
#   define ALGORITHMS_DECLSPEC_IMPORT /**/
#endif

#undef ALGORITHMS_BEGIN_NAMESPACE
#undef ALGORITHMS_END_NAMESPACE
#undef ALGORITHMS_USING
#if (__GNUC__ == 2) && (__GNUC_MINOR__ < 9)
#	define ALGORITHMS_BEGIN_NAMESPACE(x)
#	define ALGORITHMS_END_NAMESPACE
#	define ALGORITHMS_USING(x)
#else
#	define ALGORITHMS_BEGIN_NAMESPACE(ns) namespace ns{
#	define ALGORITHMS_END_NAMESPACE }
#	define ALGORITHMS_USING(ns) using ns
#endif

#ifndef FUZZY_IS_NULL
#define FUZZY_IS_NULL(dv) (abs(dv) <= 0.000000000001)
#endif

#endif
