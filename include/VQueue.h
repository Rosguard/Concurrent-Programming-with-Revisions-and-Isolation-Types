#pragma once

#include "VDataStructure.h"
#include <queue>

template <class T> class VQueue : VDataStructure<std::queue<T> > {
    private:
	void update_revision();

    public:
	void push(const T &value);
	void pop();
	T &front();
	T &back();
	[[nodiscard]] size_t size();

	void merge(const Revision *main,
		   const std::shared_ptr<Revision> &joinRev,
		   const std::shared_ptr<Segment> &join) override;
};

template <class T> void VQueue<T>::push(const T &value)
{
	DEBUG_ONLY("Push on VQueue.");

	update_revision();
	this->_versions[Revision::thread_revision()->current()->version()]
		.value()
		.push(value);
}

template <class T> void VQueue<T>::pop()
{
	DEBUG_ONLY("Pop from VQueue.");

	update_revision();
	return this
		->_versions[Revision::thread_revision()->current()->version()]
		.value()
		.pop();
}

template <class T> T &VQueue<T>::front()
{
	update_revision();
	return this->get().front();
}

template <class T> T &VQueue<T>::back()
{
	update_revision();
	return this->get().back();
}

template <class T> size_t VQueue<T>::size()
{
	const auto &value = this->get(Revision::thread_revision());
	return value.has_value() ? value.value().size() : 0;
}

template <class T>
void VQueue<T>::merge(const Revision *main,
		      const std::shared_ptr<Revision> &joinRev,
		      const std::shared_ptr<Segment> &join)
{
	DEBUG_ONLY("Try merge VQueue.");

	if (this->get_last_modified_segment(joinRev) == join) {
		DEBUG_ONLY("Make merge VQueue.");

		std::queue<T> original_queue =
			this->get_parent(joinRev).value_or(std::queue<T>());
		std::queue<T> main_queue =
			this->_versions[main->current()->version()].value();
		std::queue<T> current_queue =
			this->_versions[join->version()].value();

		std::queue<T> &new_queue =
			this->_versions[main->current()->version()].value();

		while (!original_queue.empty()) {
			while (current_queue.front() !=
				       original_queue.front() &&
			       !original_queue.empty()) {
				original_queue.pop();
			}

			if (current_queue.front() ==
				    original_queue.front() &&
			    !original_queue.empty()) {
				current_queue.pop();
			}
		}

		while (!current_queue.empty()) {
			new_queue.push(current_queue.front());
			current_queue.pop();
		}
	}
}

template <class T> void VQueue<T>::update_revision()
{
	const auto r = Revision::thread_revision();
	if (!this->_versions[r->current()->version()]) {
		DEBUG_ONLY("Record write to Segment: " + r->current()->dump() +
			   " at pos: " +
			   std::to_string(r->current()->written().size()));
		r->current()->written().push_back(this);

		// copy previous queue
		this->_versions[r->current()->version()] =
			this->get(Revision::thread_revision())
				.value_or(std::queue<T>());
	}
}