#pragma once

#include "VDataStructure.h"
#include <stack>

template <class T> class VStack : VDataStructure<std::stack<T> > {
    public:
	void push(const T &value);
	void pop();
	T &top();
	[[nodiscard]] size_t size() const;
};
