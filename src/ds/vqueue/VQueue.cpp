#include "../../../include/VQueue.h"

template <class T> void VQueue<T>::push(const T &value)
{
	this->get().push(value);
}

template <class T> void VQueue<T>::pop()
{
	this->get().pop();
}

template <class T> T &VQueue<T>::front()
{
	return this->get().front();
}

template <class T> size_t VQueue<T>::size() const
{
	return this->get().size();
}