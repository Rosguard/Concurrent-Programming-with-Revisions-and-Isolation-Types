#pragma once

#include "VDataStructure.h"
#include <stack>

// Only C++11 conformance.
// Notice that there is no const methods!
// TODO: rewrite on std::vector
template <class T> class VStack : public VDataStructure<std::stack<T> > {
    protected:
	// TODO: seriously?
	using VDataStructure<std::stack<T> >::update_revision;
	using VDataStructure<std::stack<T> >::get;
	using VDataStructure<std::stack<T> >::get_last_modified_segment;
	using VDataStructure<std::stack<T> >::get_parent_data;
	using VDataStructure<std::stack<T> >::_dummy;
	using VDataStructure<std::stack<T> >::get_guarantee;

	static std::stack<T> reverse(std::stack<T> orig);

    public:
	// TODO: constructors, destructors, operator=, emplace, https://en.cppreference.com/w/cpp/container/stack

	// TODO: push with &&
	void push(const T &value);
	void pop();
	// TODO: top with non const_reference
	const T &top();

	typename std::stack<T>::size_type size();
	bool empty();

	void swap(VStack<T> &other);

	void merge(const Revision *main,
		   const std::shared_ptr<Revision> &joinRev,
		   const std::shared_ptr<Segment> &join) override;
};

template <class T> std::stack<T> VStack<T>::reverse(std::stack<T> orig)
{
	std::stack<T> result;
	while (!orig.empty()) {
		result.push(orig.top());
		orig.pop();
	}

	return result;
}

template <class T> void VStack<T>::push(const T &value)
{
	get_guarantee(Revision::thread_revision().get()).push(value);
}

template <class T> void VStack<T>::pop()
{
	get_guarantee(Revision::thread_revision().get()).pop();
}

template <class T> const T &VStack<T>::top()
{
	const auto &val = get(Revision::thread_revision().get());
	return val ? val.value().top() :
			   _dummy.top(); // guarantee stack semantics
}

template <class T> typename std::stack<T>::size_type VStack<T>::size()
{
	const auto &val = get(Revision::thread_revision().get());
	return val ? val.value().size() : 0;
}

template <class T>
void VStack<T>::merge(const Revision *main,
		      const std::shared_ptr<Revision> &joinRev,
		      const std::shared_ptr<Segment> &join)
{
	/* Merge strategy:
	 * 0. reverse all stacks
	 * 1. all common elements of main and _original_stack VStacks -> 1'
	 * 2. all common elements of current and _original_stack VStacks -> 2'
	 * 3. all common elements of 1' and 2' VStacks -> 3'
	 * 4. ~(main - 1') -> 4'
	 * 5. ~(current - 2') -> 5'
	 * 6. result stack: 3' 4' 5'
	 */
	if (get_last_modified_segment(joinRev) == join) {
		DEBUG_ONLY("Merge VStacks.");

		std::stack<T> original_stack(reverse(
			get_parent_data(joinRev.get()).value_or(_dummy)));
		std::stack<T> main_stack(reverse(get(main).value_or(_dummy)));
		std::stack<T> current_stack(
			reverse(get(join).value_or(_dummy)));

		std::stack<T> &new_stack = get_guarantee(main);

		// clear current version
		std::stack<T> empty;
		new_stack.swap(empty);

		// calculate 3'
		while (!main_stack.empty() && !current_stack.empty() &&
		       !original_stack.empty() &&
		       main_stack.top() == current_stack.top() &&
		       current_stack.top() == original_stack.top()) {
			new_stack.push(current_stack.top());

			main_stack.pop();
			current_stack.pop();
			original_stack.pop();
		}

		std::stack<T> original_stack_copy = original_stack;
		// calculate 4'
		while (!main_stack.empty() && !original_stack.empty() &&
		       main_stack.top() == original_stack.top()) {
			main_stack.pop();
			original_stack.pop();
		}
		// calculate 5'
		while (!current_stack.empty() && !original_stack_copy.empty() &&
		       current_stack.top() == original_stack_copy.top()) {
			current_stack.pop();
			original_stack_copy.pop();
		}
		// gather stacks
		while (!main_stack.empty()) {
			new_stack.push(main_stack.top());
			main_stack.pop();
		}
		while (!current_stack.empty()) {
			new_stack.push(current_stack.top());
			current_stack.pop();
		}
	}
}

template <class T> bool VStack<T>::empty()
{
	return size() == 0;
}

template <class T> void VStack<T>::swap(VStack<T> &other)
{
	get_guarantee(Revision::thread_revision().get())
		.swap(other.get_guarantee(Revision::thread_revision().get()));
}
