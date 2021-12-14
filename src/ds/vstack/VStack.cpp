#include "../../../include/VStack.h"

template <class T> void VStack<T>::push(const T &value)
{
	this->get().push(value);
}

template <class T> void VStack<T>::pop()
{
	this->get().pop();
}

template <class T> T &VStack<T>::top()
{
	return this->get().top();
}

template <class T> size_t VStack<T>::size() const
{
	return this->get().size();
}
