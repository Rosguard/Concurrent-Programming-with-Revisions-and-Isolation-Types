#include "VDataStructure.h"

template <class T>
void VDataStructure<T>::release(const std::shared_ptr<Segment> &release)
{
	_versions[release->version()] = std::nullopt;
}

template <class T>
void VDataStructure<T>::collapse(const std::shared_ptr<Revision> &main,
				 const std::shared_ptr<Segment> &parent)
{
	if (!_versions[main->current()->version()]) {
		set(main, _versions[parent->version()]);
	}
	_versions[parent->version()] = std::nullopt;
}

template <class T>
void VDataStructure<T>::set(const std::shared_ptr<Revision> &r, const T &value)
{
	if (!_versions[r->current()->version()]) {
		r->current()->written().push_back(shared_from_this());
	}
	_versions[r->current()->version()] = value;
}

template <class T>
void VDataStructure<T>::merge(const std::shared_ptr<Revision> &main,
			      const std::shared_ptr<Revision> &joinRev,
			      const std::shared_ptr<Segment> &join)
{
	auto s = joinRev->current();

	while (!_versions[s->version()]) {
		s = s->parent();
	}

	// TODO: possible error.
	if (s == join) {
		set(main, _versions[join->version()]);
	}
}

template <class T> T &VDataStructure<T>::get(const std::shared_ptr<Revision> &r)
{
	auto s = r->current();
	while (!_versions[s->version()]) {
		s = s->parent();
	}

	return _versions[s->version()];
}

template <class T> void VDataStructure<T>::set(const T &value)
{
	set(Revision::thread_revision(), value);
}

template <class T> T &VDataStructure<T>::get()
{
	return get(Revision::thread_revision());
}
