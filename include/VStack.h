#pragma once

#include "VDataStructure.h"
#include <stack>

template <class T> class VStack : VDataStructure<std::stack<T> > {
    private:
	void update_revision();

    public:
	void push(const T &value);
	void pop();
	T &top();
	[[nodiscard]] size_t size();

	void merge(const Revision *main,
		   const std::shared_ptr<Revision> &joinRev,
		   const std::shared_ptr<Segment> &join) override;
};

template <class T> void VStack<T>::push(const T &value)
{
	DEBUG_ONLY("Push on VStack.");

	update_revision();
	this->_versions[Revision::thread_revision()->current()->version()]
		.value()
		.push(value);
}

template <class T> void VStack<T>::pop()
{
	DEBUG_ONLY("Pop from VStack.");

	update_revision();
	return this
		->_versions[Revision::thread_revision()->current()->version()]
		.value()
		.pop();
}

template <class T> T &VStack<T>::top()
{
	update_revision();
	return this->get().top();
}

template <class T> size_t VStack<T>::size()
{
	const auto &value = this->get(Revision::thread_revision());
	return value.has_value() ? value.value().size() : 0;
}

template <class T> std::stack<T> reverse(std::stack<T> &orig)
{
	std::stack<T> result;
	while (!orig.empty()) {
		result.push(orig.top());
		orig.pop();
	}

	return result;
}

template <class T>
void VStack<T>::merge(const Revision *main,
		      const std::shared_ptr<Revision> &joinRev,
		      const std::shared_ptr<Segment> &join)
{
	DEBUG_ONLY("Try merge VStack.");

	/* Merge strategy:
	 * 0. reverse all stacks
	 * 1. all common elements of main and _original_stack VStacks -> 1'
	 * 2. all common elements of current and _original_stack VStacks -> 2'
	 * 3. all common elements of 1' and 2' VStacks -> 3'
	 * 4. ~(main - 1') -> 4'
	 * 5. ~(current - 2') -> 5'
	 * 6. result stack: 3' 4' 5'
	 */
	if (this->get_last_modified_segment(joinRev) == join) {
		DEBUG_ONLY("Make merge VStack.");

		std::stack<T> original_stack =
			this->get_parent(joinRev).value_or(std::stack<T>());
		original_stack = reverse(original_stack);
		std::stack<T> main_stack = reverse(
			this->_versions[main->current()->version()].value());
		std::stack<T> current_stack =
			reverse(this->_versions[join->version()].value());

		std::stack<T> &new_stack =
			this->_versions[main->current()->version()].value();
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

template <class T> void VStack<T>::update_revision()
{
	const auto r = Revision::thread_revision();
	if (!this->_versions[r->current()->version()]) {
		DEBUG_ONLY("Record write to Segment: " + r->current()->dump() +
			   " at pos: " +
			   std::to_string(r->current()->written().size()));
		r->current()->written().push_back(this);

		// copy previous stack
		this->_versions[r->current()->version()] =
			this->get(Revision::thread_revision())
				.value_or(std::stack<T>());
	}
}
