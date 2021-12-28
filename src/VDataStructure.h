#pragma once

#include "versioned/Versioned.h"
#include "segment/Segment.h"
#include "revision/Revision.h"
#include <unordered_map>
#include <optional>

template <class T> class VDataStructure : public Versioned {
    protected:
	T _dummy; // requires default constructor
	std::unordered_map<int, std::optional<T> > _versions;

	// useful to update _versions
	void update_revision(const Revision *r, bool force_init = true);

	void set(const Revision *r, const T &value);

	// get value
	std::optional<T> &get(const Revision *r);
	// guarantee value for revision
	T &get_guarantee(const Revision *r);
	// get exact current version of the data or create dummy
	T &get_no_update(const Revision *r);
	T &get_no_update(const std::shared_ptr<Segment> &s);
	// get parent version of the data
	std::optional<T> &get_parent_data(const Revision *r);

	[[nodiscard]] std::shared_ptr<Segment>
	get_last_modified_segment(const std::shared_ptr<Revision> &r) const;

	virtual T &get();

    public:
	void release(const Segment *release) override;

	void collapse(const Revision *main,
		      const std::shared_ptr<Segment> &parent) override;

	~VDataStructure() override;
};

template <class T> T &VDataStructure<T>::get()
{
	return get(Revision::thread_revision().get()).value();
}

template <class T> void VDataStructure<T>::release(const Segment *release)
{
	_versions[release->version()] = std::nullopt;
}

// TODO: need rewrite for each ds?
template <class T>
void VDataStructure<T>::collapse(const Revision *main,
				 const std::shared_ptr<Segment> &parent)
{
	// TODO: possible error?
	if (!_versions[main->current()->version()]) {
		set(main, _versions[parent->version()].value());
	}
	_versions[parent->version()] = std::nullopt;
}

template <class T>
void VDataStructure<T>::set(const Revision *r, const T &value)
{
	update_revision(r, false);
	_versions[r->current()->version()] = value;
}

template <class T> std::optional<T> &VDataStructure<T>::get(const Revision *r)
{
	if (_versions[r->current()->version()]) {
		return _versions[r->current()->version()];
	}

	return get_parent_data(r);
}

template <class T> VDataStructure<T>::~VDataStructure()
{
	// delete all references to this data, because we are leaving context
	auto s = Revision::thread_revision()->current();

	auto to_delete =
		std::remove(s->written().begin(), s->written().end(), this);
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
std::optional<T> &VDataStructure<T>::get_parent_data(const Revision *r)
{
	if (!r->current()->parent()) {
		return _versions[r->current()->version()]; // nullopt
	}

	auto s = r->current()->parent();
	while (s->parent() && !_versions[s->version()]) {
		s = s->parent();
	}

	return _versions[s->version()];
}

template <class T>
void VDataStructure<T>::update_revision(const Revision *r, bool force_init)
{
	if (!this->_versions[r->current()->version()]) {
		DEBUG_ONLY("Record write to Segment: " + r->current()->dump());
		r->current()->written().push_back(this);

		if (force_init) {
			this->_versions[r->current()->version()] =
				this->get(Revision::thread_revision().get())
					.value_or(_dummy);
		}
	}
}

template <class T> T &VDataStructure<T>::get_no_update(const Revision *r)
{
	return get_no_update(r->current());
}

template <class T> T &VDataStructure<T>::get_guarantee(const Revision *r)
{
	update_revision(r, true);
	return get(r).value();
}

template <class T>
T &VDataStructure<T>::get_no_update(const std::shared_ptr<Segment> &s)
{
	return _versions[s->version()] ? _versions[s->version()].value() :
					       _dummy;
}
