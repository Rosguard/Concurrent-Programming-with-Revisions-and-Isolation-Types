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
	std::optional<T> &get(const std::shared_ptr<Revision> &r);
	std::optional<T> &
	get_parent(const std::shared_ptr<Revision>
			   &r); // get parent version of the data

	[[nodiscard]] std::shared_ptr<Segment>
	get_last_modified_segment(const std::shared_ptr<Revision> &r) const;

    public:
	void set(const T &value);
	T &get();

	void release(const Segment *release) override;

	void collapse(const Revision *main,
		      const std::shared_ptr<Segment> &parent) override;

	void merge(const Revision *main,
		   const std::shared_ptr<Revision> &joinRev,
		   const std::shared_ptr<Segment> &join) override;

	virtual ~VDataStructure();
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
		DEBUG_ONLY("Record set to Segment: " + r->current()->dump() +
			   " at pos: " +
			   std::to_string(r->current()->written().size()));
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

	if (get_last_modified_segment(joinRev) == join) {
		DEBUG_ONLY("Make merge");
		set(main, _versions[join->version()].value());
	}
}

template <class T>
std::optional<T> &VDataStructure<T>::get(const std::shared_ptr<Revision> &r)
{
	if (_versions[r->current()->version()]) {
		return _versions[r->current()->version()];
	}

	return get_parent(r);
}

template <class T> void VDataStructure<T>::set(const T &value)
{
	set(Revision::thread_revision().get(), value);
}

template <class T> T &VDataStructure<T>::get()
{
	return get(Revision::thread_revision()).value();
}

template <class T> VDataStructure<T>::~VDataStructure()
{
	// delete all references to this data, because we are leaving context
	auto s = Revision::thread_revision()->current();
	if (s->written().empty()) {
		return;
	}
	auto to_delete = std::remove_if(
		s->written().begin(), s->written().end(),
		[this](const Versioned *ptr) { return ptr == this; });
	if (to_delete != s->written().end()) {
		s->written().erase(to_delete);
	}
}

template <class T>
std::shared_ptr<Segment> VDataStructure<T>::get_last_modified_segment(
	const std::shared_ptr<Revision> &r) const
{
	auto s = r->current();

	while (_versions.find(s->version()) == _versions.end() ||
	       !_versions.at(s->version())) {
		s = s->parent();
	}

	return s;
}

template <class T>
std::optional<T> &
VDataStructure<T>::get_parent(const std::shared_ptr<Revision> &r)
{
	if (!r->current()->parent()) {
		return _versions[r->current()->version()]; // nullopt
	}

	auto s = r->current()->parent();
	while (!_versions[s->version()]) {
		if (!s->parent()) {
			break; // nullopt, not found
		}
		s = s->parent();
	}

	return _versions[s->version()];
}
