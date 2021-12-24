#pragma once

#include "ds/VDataStructure.h"
#include <queue>

template <class T> class VQueue : VDataStructure<std::queue<T> > {
    public:
	void push(const T &value);
	void pop();
	T &front();
	[[nodiscard]] size_t size() const;
};