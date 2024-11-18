#pragma once
#include"global.hpp"

namespace std
{
	template <class InputIterator,typename T>
	InputIterator find(InputIterator first, InputIterator last, const T& value)
	{
		//
		// һ������*first != value��������value != *first
		//
		while (first != last && *first != value)
			first++;

		return first;
	}






































}