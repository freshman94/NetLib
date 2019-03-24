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
ʹ��implicit_cast��Ϊstatic_cast��const_cast�İ�ȫ�汾��
��������ת��������ָ��������ָ��ת��Ϊָ������ָ�룩���߽�Foo*ת��Ϊconst Foo*
ʹ��implicit_castʱ������������ǿ��ת���Ƿ�ȫ��
������ʽ��implicit_casts�Ǳ�Ҫ�ģ���ΪC ++��Ҫ��ȷ����ƥ�䡣

From���Ϳ����ƶϣ����ʹ�õ��﷨��static_cast��ͬ��implicit_cast <ToType>��expr��
implicit_cast����ΪC ++��׼���һ����
*/
template<typename To, typename From>
inline To implicit_cast(From const& f) {
	return f;
}

/*
static_cast�İ�ȫ�汾�����ڱ�֤����ת���İ�ȫ�ԡ�������֤To����ȷʵ��From���͵������ࣩ

����ģʽ�У�����ʹ��Double-check��֤����ת���ǺϷ��ģ������Ϸ�������die����
����ģʽ�У�ʹ��static_castִ������ת������ˣ��ڵ���ģʽ�б�֤ת���ĺϷ����Ǻ���Ҫ�ġ�

ʹ��dynamic_castִ��RTTI�ǲ���ȡ�ģ��磺
if (dynamic_cast<Subclass1>(foo)) HandleASubclass1Object(foo);
if (dynamic_cast<Subclass2>(foo)) HandleASubclass2Object(foo);
Ӧʹ�������ķ���

ʹ���﷨��down_cast<T*>(foo)	ע�⣺������ָ��
*/

template<typename To, typename From>
inline To down_cast(From* f) {
	/*
	���ڱ�֤To����ȷʵ��From���͵������ࡣ�����ڱ����ڽ������ͼ�飬�������ھ����Ż��������������Ŀ�����
	*/
	if (false)	implicit_cast<From*, To>(0);

#if !defined(NDEBUG) && !defined(GOOGLE_PROTOBUF_NO_RTTI)
	assert(f == NULL || dynamic_cast<To>(f) != NULL);
#endif
	return static_cast<To>(f);
}
