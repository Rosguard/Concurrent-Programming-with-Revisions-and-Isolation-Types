#pragma once

#include "VDataStructure.h"
#include <queue>

template <class T> class VQueue : VDataStructure<std::queue<T> > {
    private:
	void update_revision();
	bool isSubQueue(const std::queue<T> &originalQueue,
			const std::queue<T> &checkQueue);
	size_t subQueueLength(const std::queue<T> &originalQueue,
			      const std::queue<T> &checkQueue);

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

		bool isMainSub = isSubQueue(original_queue, main_queue);
		bool isCurrentSub = isSubQueue(original_queue, current_queue);

		if (!isMainSub) {
			if (!isCurrentSub) {
				//If all elements of main and current aren't elements from original queue
				//res: main + current
				while (!current_queue.empty()) {
					new_queue.push(current_queue.front());
					current_queue.pop();
				}
			} else {
				//If all elements of main aren't elements from original queue
				//And some elements of current are elements from original queue
				//res: main + current(without original elements)
				while (isSubQueue(original_queue,
						  current_queue)) {
					current_queue.pop();
				}
				while (!current_queue.empty()) {
					new_queue.push(current_queue.front());
					current_queue.pop();
				}
			}
		} else {
			if (!isCurrentSub) {
				//If all elements of current aren't elements from original queue
				//And some elements of main are elements from original queue
				//res: main(without original elements) + current
				while (isMainSub) {
					main_queue.pop();

					isMainSub = isSubQueue(original_queue,
							       main_queue);
				}
				while (!current_queue.empty()) {
					new_queue.push(current_queue.front());
					current_queue.pop();
				}
			} else {
				//If some elements of main and current are elements from original queue
				size_t subMainLen = subQueueLength(
					original_queue, main_queue);
				size_t subCurrentLen = subQueueLength(
					original_queue, current_queue);

				while (subMainLen > subCurrentLen &&
				       !main_queue.empty() &&
				       !new_queue.empty()) {
					main_queue.pop();
					new_queue.pop();

					subMainLen--;
				}
				while (subMainLen < subCurrentLen &&
				       !current_queue.empty()) {
					current_queue.pop();

					subCurrentLen--;
				}
				while (isCurrentSub && !current_queue.empty()) {
					current_queue.pop();

					isCurrentSub = isSubQueue(
						original_queue, current_queue);
				}
				while (!current_queue.empty()) {
					new_queue.push(current_queue.front());
					current_queue.pop();
				}
			}
		}
	}
}

template <class T>
bool VQueue<T>::isSubQueue(const std::queue<T> &originalQueue,
			   const std::queue<T> &checkQueue)
{
	std::queue<T> original_queue_copy = originalQueue;
	std::queue<T> check_queue_copy = checkQueue;

	while (original_queue_copy.front() != check_queue_copy.front() &&
	       !original_queue_copy.empty()) {
		original_queue_copy.pop();
	}
	if (original_queue_copy.empty()) {
		return false;
	}
	while (!original_queue_copy.empty() && !check_queue_copy.empty()) {
		if (original_queue_copy.front() == check_queue_copy.front()) {
			original_queue_copy.pop();
			check_queue_copy.pop();
		} else {
			return false;
		}
	}
	return true;
}

template <class T>
size_t VQueue<T>::subQueueLength(const std::queue<T> &originalQueue,
				 const std::queue<T> &checkQueue)
{
	size_t sub_size = 0;
	std::queue<T> original_queue_copy = originalQueue;
	std::queue<T> check_queue_copy = checkQueue;

	while (isSubQueue(original_queue_copy, check_queue_copy)) {
		original_queue_copy.pop();
		check_queue_copy.pop();
		sub_size++;
	}

	return sub_size;
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