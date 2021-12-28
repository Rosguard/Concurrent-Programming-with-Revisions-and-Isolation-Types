#pragma once

#include "VDataStructure.h"

template <class T> class VBasicType : public VDataStructure<T> {
    protected:
	// TODO: seriously?
	using VDataStructure<T>::set;

    public:
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
	if (this->get_last_modified_segment(joinRev) == join) {
		DEBUG_ONLY("Merge VBasicTypes.");
		set(main, this->_versions[join->version()].value());
	}
}
