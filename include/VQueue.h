#pragma once

#include "VDataStructure.h"
#include <queue>

// Only C++11 conformance.
// Notice that there is no const methods!
// TODO: rewrite on std::vector
template <class T> class VQueue : public VDataStructure<std::queue<T> > {
    protected:
	using VDataStructure<std::queue<T> >::update_revision;
	using VDataStructure<std::queue<T> >::get;
	using VDataStructure<std::queue<T> >::get_last_modified_segment;
	using VDataStructure<std::queue<T> >::get_parent_data;
	using VDataStructure<std::queue<T> >::_dummy;
	using VDataStructure<std::queue<T> >::get_guarantee;

	static bool is_sub_queue(const std::queue<T> &original_queue,
				 const std::queue<T> &check_queue);
	static size_t sub_queue_length(const std::queue<T> &original_queue,
				       const std::queue<T> &check_queue);

    public:
	// TODO: constructors, destructors, operator=, emplace (https://en.cppreference.com/w/cpp/container/queue)

	// TODO: push with &&
	void push(const T &value);
	void pop();

	// TODO: front and back with non const_reference
	const T &front();
	const T &back();

	typename std::queue<T>::size_type size();
	bool empty();

	void swap(VQueue &other);

	void merge(const Revision *main,
		   const std::shared_ptr<Revision> &joinRev,
		   const std::shared_ptr<Segment> &join) override;
};

template <class T> void VQueue<T>::push(const T &value)
{
	get_guarantee(Revision::thread_revision().get()).push(value);
}

template <class T> void VQueue<T>::pop()
{
	get_guarantee(Revision::thread_revision().get()).pop();
}

template <class T> const T &VQueue<T>::front()
{
	const auto &val = get(Revision::thread_revision().get());
	return val ? val.value().front() :
			   _dummy.front(); // guarantee queue semantics
}

template <class T> const T &VQueue<T>::back()
{
	const auto &val = get(Revision::thread_revision().get());
	return val ? val.value().back() :
			   _dummy.back(); // guarantee queue semantics
}

template <class T> typename std::queue<T>::size_type VQueue<T>::size()
{
	const auto &value = this->get(Revision::thread_revision().get());
	return value ? value.value().size() : 0;
}

template <class T>
void VQueue<T>::merge(const Revision *main,
		      const std::shared_ptr<Revision> &joinRev,
		      const std::shared_ptr<Segment> &join)
{
	if (this->get_last_modified_segment(joinRev) == join) {
		DEBUG_ONLY("Merge VQueue.");

		std::queue<T> original_queue =
			get_parent_data(joinRev.get()).value_or(_dummy);
		std::queue<T> main_queue = get(main).value_or(_dummy);
		std::queue<T> current_queue = get(join).value_or(_dummy);

		std::queue<T> &new_queue = get_guarantee(main);

		bool isMainSub = is_sub_queue(original_queue, main_queue);
		bool isCurrentSub = is_sub_queue(original_queue, current_queue);

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
				while (is_sub_queue(original_queue,
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

					isMainSub = is_sub_queue(original_queue,
								 main_queue);
				}
				while (!current_queue.empty()) {
					new_queue.push(current_queue.front());
					current_queue.pop();
				}
			} else {
				//If some elements of main and current are elements from original queue
				size_t subMainLen = sub_queue_length(
					original_queue, main_queue);
				size_t subCurrentLen = sub_queue_length(
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

					isCurrentSub = is_sub_queue(
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
bool VQueue<T>::is_sub_queue(const std::queue<T> &original_queue,
			     const std::queue<T> &check_queue)
{
	std::queue<T> original_queue_copy = original_queue;
	std::queue<T> check_queue_copy = check_queue;

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
size_t VQueue<T>::sub_queue_length(const std::queue<T> &original_queue,
				   const std::queue<T> &check_queue)
{
	size_t sub_size = 0;
	std::queue<T> original_queue_copy = original_queue;
	std::queue<T> check_queue_copy = check_queue;

	while (is_sub_queue(original_queue_copy, check_queue_copy)) {
		original_queue_copy.pop();
		check_queue_copy.pop();
		sub_size++;
	}

	return sub_size;
}
