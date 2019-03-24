#pragma once

#include <stdint.h>
#include <string.h>	//memset
#include <string>

#ifndef NDEBUG
#include <assert.h>
#endif

using std::string;
inline void memZero(void* p, size_t n) {
	memset(p, 0, n);
}

/*
使用implicit_cast作为static_cast或const_cast的安全版本。
用于向上转换（即将指向派生的指针转换为指向基类的指针）或者将Foo*转换为const Foo*
使用implicit_cast时，编译器会检查强制转换是否安全。
这种显式的implicit_casts是必要的，因为C ++需要精确类型匹配。

From类型可以推断，因此使用的语法与static_cast相同：implicit_cast <ToType>（expr）
implicit_cast将成为C ++标准库的一部分
*/
template<typename To, typename From>
inline To implicit_cast(From const& f) {
	return f;
}

/*
static_cast的安全版本，用于保证向下转换的安全性。（即保证To类型确实是From类型的派生类）

调试模式中，我们使用Double-check保证向下转换是合法的（若不合法，程序die）。
正常模式中，使用static_cast执行向下转换。因此，在调试模式中保证转换的合法性是很重要的。

使用dynamic_cast执行RTTI是不可取的，如：
if (dynamic_cast<Subclass1>(foo)) HandleASubclass1Object(foo);
if (dynamic_cast<Subclass2>(foo)) HandleASubclass2Object(foo);
应使用其它的方法

使用语法：down_cast<T*>(foo)	注意：仅接受指针
*/

template<typename To, typename From>
inline To down_cast(From* f) {
	/*
	用于保证To类型确实是From类型的派生类。仅会在编译期进行类型检查，在运行期经过优化，不会产生额外的开销。
	*/
	if (false)	implicit_cast<From*, To>(0);

#if !defined(NDEBUG) && !defined(GOOGLE_PROTOBUF_NO_RTTI)
	assert(f == NULL || dynamic_cast<To>(f) != NULL);
#endif
	return static_cast<To>(f);
}
