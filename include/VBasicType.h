#pragma once

#include "VDataStructure.h"

template <class T> class VBasicType : public VDataStructure<T> {
    protected:
	using VDataStructure<T>::set;
	using VDataStructure<T>::get_last_modified_segment;
	using VDataStructure<T>::_versions;
	using VDataStructure<T>::_merge_strategy;
	using VDataStructure<T>::_dummy;
	using VDataStructure<T>::get_parent_data;
	using VDataStructure<T>::get;
	using VDataStructure<T>::call_strategy;

    public:
	VBasicType() = default;
	explicit VBasicType(
		const std::function<T(const T &, const T &, const T &)> &f)
		: VDataStructure<T>(f)
	{
	}

	void set(const T &value);
	T &get();

	void merge(const Revision *main,
		   const std::shared_ptr<Revision> &joinRev,
		   const std::shared_ptr<Segment> &join) override;
};

template <class T> void VBasicType<T>::set(const T &value)
{
	set(Revision::thread_revision().get(), value);
}

template <class T> T &VBasicType<T>::get()
{
	return VDataStructure<T>::get();
}

template <class T>
void VBasicType<T>::merge(const Revision *main,
			  const std::shared_ptr<Revision> &joinRev,
			  const std::shared_ptr<Segment> &join)
{
	if (get_last_modified_segment(joinRev) == join) {
		DEBUG_ONLY("Merge VBasicTypes.");
		if (!_merge_strategy) {
			set(main, _versions[join->version()].value());
		} else {
			set(main, call_strategy(main, joinRev, join));
		}
	}
}
