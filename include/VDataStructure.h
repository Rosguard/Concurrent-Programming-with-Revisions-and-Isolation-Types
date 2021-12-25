#pragma once

#include "versioned/Versioned.h"
#include "segment/Segment.h"
#include "revision/Revision.h"
#include <unordered_map>
#include <optional>

template <class T> class VDataStructure : public Versioned {
    protected:
	std::unordered_map<int, std::optional<T> > _versions;

	void set(const Revision *r, const T &value);
	T &get(const std::shared_ptr<Revision> &r);

    public:
	void set(const T &value);
	T &get();

	void release(const Segment *release) override;

	void collapse(const Revision *main,
		      const std::shared_ptr<Segment> &parent) override;

	void merge(const Revision *main,
		   const std::shared_ptr<Revision> &joinRev,
		   const std::shared_ptr<Segment> &join) override;
};

template <class T> void VDataStructure<T>::release(const Segment *release)
{
	_versions[release->version()] = std::nullopt;
}

template <class T>
void VDataStructure<T>::collapse(const Revision *main,
				 const std::shared_ptr<Segment> &parent)
{
	if (!_versions[main->current()->version()]) {
		set(main, _versions[parent->version()].value());
	}
	_versions[parent->version()] = std::nullopt;
}

template <class T>
void VDataStructure<T>::set(const Revision *r, const T &value)
{
	DEBUG_ONLY("Set value");

	if (!_versions[r->current()->version()]) {
		DEBUG_ONLY("Record set to Segment: " +
			   r->current()->version_to_string());
		r->current()->written().push_back(this);
	}
	_versions[r->current()->version()] = value;
}

template <class T>
void VDataStructure<T>::merge(const Revision *main,
			      const std::shared_ptr<Revision> &joinRev,
			      const std::shared_ptr<Segment> &join)
{
	DEBUG_ONLY("Try merge");

	auto s = joinRev->current();

	while (!_versions[s->version()]) {
		s = s->parent();
	}

	// TODO: possible error.
	if (s == join) {
		DEBUG_ONLY("Make merge");
		set(main, _versions[join->version()].value());
	}
}

template <class T> T &VDataStructure<T>::get(const std::shared_ptr<Revision> &r)
{
	auto s = r->current();
	while (!_versions[s->version()]) {
		s = s->parent();
	}

	return _versions[s->version()].value();
}

template <class T> void VDataStructure<T>::set(const T &value)
{
	set(Revision::thread_revision().get(), value);
}

template <class T> T &VDataStructure<T>::get()
{
	return get(Revision::thread_revision());
}
