#pragma once

#include "VDataStructure.h"
#include <set>
#include <algorithm>

// Only C++11 conformance.
// Note that there is no const member functions!
template <class T> class VSet : VDataStructure<std::set<T> > {
    protected:
	static std::set<T> intersect(const std::set<T> &s1,
				     const std::set<T> &s2);

	static std::set<T> difference(const std::set<T> &s1,
				      const std::set<T> &s2);

    public:
	// TODO: seriously?
	using VDataStructure<std::set<T> >::update_revision;
	using VDataStructure<std::set<T> >::get_guarantee;
	using VDataStructure<std::set<T> >::get;
	using VDataStructure<std::set<T> >::get_last_modified_segment;
	using VDataStructure<std::set<T> >::get_parent_data;
	using VDataStructure<std::set<T> >::_dummy;

	typename std::set<T>::iterator begin();
	typename std::set<T>::iterator end();

	// TODO: constructors, destructors, operator=, get_allocator
	// TODO: all iterators (https://en.cppreference.com/w/cpp/container/set)
	bool empty();
	typename std::set<T>::size_type size();
	void clear();
	typename std::set<T>::size_type max_size();

	// TODO: other insert types (https://en.cppreference.com/w/cpp/container/set/insert)
	// TODO: emplace, emplace_hint
	std::pair<typename std::set<T>::iterator, bool> insert(const T &value);

	// TODO: other erase types (https://en.cppreference.com/w/cpp/container/set/erase)
	typename std::set<T>::size_type erase(const T &key);

	void swap(VSet<T> &other);

	typename std::set<T>::size_type count(const T &key);

	// TODO: find for const iterator
	typename std::set<T>::iterator find(const T &key);

	// TODO: equal_range for const iterator
	std::pair<typename std::set<T>::iterator, typename std::set<T>::iterator>
	equal_range(const T &key);

	// TODO: lower_bound for const iterator
	typename std::set<T>::iterator lower_bound(const T &key);

	// TODO: upper_bound for const iterator
	typename std::set<T>::iterator upper_bound(const T &key);

	typename std::set<T>::key_compare key_comp();
	typename std::set<T>::value_compare value_comp();

	void merge(const Revision *main,
		   const std::shared_ptr<Revision> &joinRev,
		   const std::shared_ptr<Segment> &join) override;
};

template <class T> bool VSet<T>::empty()
{
	return size() == 0;
}

template <class T> typename std::set<T>::size_type VSet<T>::size()
{
	const auto &value = get(Revision::thread_revision().get());
	return value ? value.value().size() : 0;
}

template <class T> void VSet<T>::clear()
{
	get_guarantee(Revision::thread_revision().get()).clear();
}

template <class T> typename std::set<T>::size_type VSet<T>::max_size()
{
	const auto &value = get(Revision::thread_revision().get());
	return value ? value.value().max_size() :
			     std::set<T>()
			       .max_size(); // TODO: what is the default value?
}

template <class T>
std::pair<typename std::set<T>::iterator, bool> VSet<T>::insert(const T &value)
{
	return get_guarantee(Revision::thread_revision().get()).insert(value);
}

template <class T> typename std::set<T>::size_type VSet<T>::erase(const T &key)
{
	return get_guarantee(Revision::thread_revision().get()).erase(key);
}

template <class T> void VSet<T>::swap(VSet<T> &other)
{
	get_guarantee(Revision::thread_revision().get())
		.swap(other.get_guarantee(Revision::thread_revision().get()));
}

template <class T> typename std::set<T>::size_type VSet<T>::count(const T &key)
{
	const auto &value = get(Revision::thread_revision().get());
	return value ? value.value().count(key) : 0;
}

template <class T> typename std::set<T>::iterator VSet<T>::find(const T &key)
{
	// get_guarantee because we want valid iterator
	return get_guarantee(Revision::thread_revision().get()).find(key);
}

template <class T>
std::pair<typename std::set<T>::iterator, typename std::set<T>::iterator>
VSet<T>::equal_range(const T &key)
{
	// promote data to this revision because we want valid iterator
	return get_guarantee(Revision::thread_revision().get()).equal_range(key);
}

template <class T>
typename std::set<T>::iterator VSet<T>::lower_bound(const T &key)
{
	// promote data to this revision because we want valid iterator
	return get_guarantee(Revision::thread_revision().get()).lower_bound(key);
}

template <class T>
typename std::set<T>::iterator VSet<T>::upper_bound(const T &key)
{
	// promote data to this revision because we want valid iterator
	return get_guarantee(Revision::thread_revision().get()).upper_bound(key);
}

template <class T> typename std::set<T>::key_compare VSet<T>::key_comp()
{
	const auto &value = get(Revision::thread_revision().get());
	return value ? value.value().key_comp() :
			     std::set<T>()
			       .key_comp(); // TODO: what is the default value?
}

template <class T> typename std::set<T>::value_compare VSet<T>::value_comp()
{
	const auto &value = get(Revision::thread_revision().get());
	return value ? value.value().value_compare() :
			     std::set<T>()
			       .value_compare(); // TODO: what is the default value?
}

template <class T>
void VSet<T>::merge(const Revision *main,
		    const std::shared_ptr<Revision> &joinRev,
		    const std::shared_ptr<Segment> &join)
{
	/* Merge strategy:
	 * 1. intersection of main and original sets -> 1'
	 * 2. intersection of current and original sets -> 2'
	 * 3. intersection of 1' and 2' -> 3'
	 * 4. difference of main and original sets -> 4'
	 * 5. difference of current and original sets -> 5'
	 * 6. result: 3' + 4' + 5'
	 */
	if (get_last_modified_segment(joinRev) == join) {
		DEBUG_ONLY("Merge VSets.");

		std::set<T> original_set =
			get_parent_data(joinRev.get()).value_or(_dummy);
		std::set<T> main_set = get(main).value_or(_dummy);
		std::set<T> current_set = get(join).value_or(_dummy);

		std::set<T> &new_stack = get_guarantee(main);

		// 3'
		new_stack = intersect(intersect(main_set, original_set),
				      intersect(current_set, original_set));
		// 4'
		auto difference_set =
			std::move(difference(main_set, original_set));

		new_stack.insert(difference_set.begin(), difference_set.end());

		// 5'
		difference_set =
			std::move(difference(current_set, original_set));
		new_stack.insert(difference_set.begin(), difference_set.end());
	}
}

template <class T>
std::set<T> VSet<T>::intersect(const std::set<T> &s1, const std::set<T> &s2)
{
	std::set<T> intersection;

	std::set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(),
			      std::inserter(intersection,
					    intersection.begin()));

	return intersection;
}

template <class T>
std::set<T> VSet<T>::difference(const std::set<T> &s1, const std::set<T> &s2)
{
	std::set<T> difference;

	std::set_difference(s1.begin(), s1.end(), s2.begin(), s2.end(),
			    std::inserter(difference, difference.begin()));

	return difference;
}

template <class T> typename std::set<T>::iterator VSet<T>::begin()
{
	// promote data to this revision because we want valid iterator
	return get_guarantee((Revision::thread_revision().get())).begin();
}

template <class T> typename std::set<T>::iterator VSet<T>::end()
{
	// promote data to this revision because we want valid iterator
	return get_guarantee((Revision::thread_revision().get())).end();
}
