#include "VStack.h"
#include <gtest/gtest.h>

TEST(VStackTest, push_top)
{
	VStack<int> stack;

	stack.push(1);
	ASSERT_EQ(stack.top(), 1);
}

TEST(VStackTest, push_pop_size)
{
	VStack<int> stack;

	stack.push(1);
	ASSERT_EQ(stack.size(), 1);

	stack.pop();
	ASSERT_EQ(stack.size(), 0);
}

TEST(VStackTest, empty_test)
{
	VStack<int> stack;
	ASSERT_EQ(stack.size(), 0);
	ASSERT_EQ(stack.empty(), true);

	stack.push(1);
	ASSERT_EQ(stack.size(), 1);
	ASSERT_EQ(stack.empty(), false);

	stack.pop();
	ASSERT_EQ(stack.size(), 0);
	ASSERT_EQ(stack.empty(), true);
}

TEST(VStackTest, swap_test)
{
	VStack<int> first_stack;
	ASSERT_EQ(first_stack.size(), 0);

	VStack<int> second_stack;
	ASSERT_EQ(second_stack.size(), 0);

	for (int i = 0; i < 6; ++i) {
		first_stack.push(i * 30);
	}
	ASSERT_EQ(first_stack.size(), 6);

	for (int i = 0; i < 10; ++i) {
		second_stack.push(i * 100);
	}
	ASSERT_EQ(second_stack.size(), 10);

	first_stack.swap(second_stack);

	ASSERT_EQ(second_stack.size(), 6);
	ASSERT_EQ(first_stack.size(), 10);
}

TEST(VStackTest, multithread)
{
	VStack<int> stack;

	for (int i = 0; i < 10; i++) {
		stack.push(i);
	}
	ASSERT_EQ(stack.size(), 10);

	volatile bool synchro = false;

	// 1st thread
	const auto r1 = Revision::thread_revision()->fork([&stack, &synchro]() {
		while (!synchro) {
		} // wait until 2nd thread set var

		ASSERT_EQ(stack.size(), 10);

		for (int i = 0; i < 5; i++) {
			ASSERT_EQ(stack.top(), 10 - i - 1);
			stack.pop();
		}
		ASSERT_EQ(stack.size(), 5);

		for (int i = 0; i < 5; i++) {
			stack.push(i * 3); // top: 12 9 6 3 0 4 3 2 1 0
		}
		ASSERT_EQ(stack.size(), 10);
	});

	// 2nd thread
	const auto r2 = Revision::thread_revision()->fork([&stack]() {
		ASSERT_EQ(stack.size(), 10);

		for (int i = 0; i < 3; i++) {
			ASSERT_EQ(stack.top(), 10 - i - 1);
			stack.pop();
		}
		ASSERT_EQ(stack.size(), 7);

		for (int i = 0; i < 7; i++) {
			stack.push(i *
				   10); // top:60 50 40 30 20 10 0 6 5 4 3 2 1 0
		}
		ASSERT_EQ(stack.size(), 14);
	});

	ASSERT_EQ(stack.size(), 10);

	for (int i = 0; i < 8; i++) {
		ASSERT_EQ(stack.top(), 10 - i - 1);
		stack.pop();
	}
	ASSERT_EQ(stack.size(), 2);

	/*
	 * 1. r2:		60 50 40 30 20 10 0 6 5 4 3 2 1 0
	 * 2. main:		      		      	      1 0
	 * 3. original stack:   	      9 8 7 6 5 4 3 2 1 0
	 * 3. res:      		  60 50 40 30 20 10 0 1 0
	 */

	// let first thread stop later
	Revision::thread_revision()->join(r2);
	ASSERT_EQ(stack.size(), 9);

	/*
	 * 1. r1:			     12 9 6 3 0 4 3 2 1 0
	 * 2. main:		          60 50 40 30 20 10 0 1 0
	 * 3. original stack:   	      9 8 7 6 5 4 3 2 1 0
	 * 3. res:             12 9 6 3 0 60 50 40 30 20 10 0 1 0
	 */

	synchro = true;
	Revision::thread_revision()->join(r1);
	ASSERT_EQ(stack.size(), 14);

	std::vector<int> good_answer = { 0,  1,	 0, 10, 20, 30, 40,
					 50, 60, 0, 3,	6,  9,	12 };
	while (stack.size()) {
		ASSERT_EQ(stack.top(), good_answer.back());
		stack.pop();
		good_answer.pop_back();
	}
}

TEST(VStackTest, nothing_doing)
{
	VStack<int> stack;
	ASSERT_EQ(stack.size(), 0);

	// 1st thread
	const auto r1 = Revision::thread_revision()->fork(
		[&stack]() { ASSERT_EQ(stack.size(), 0); });

	// 2nd thread
	const auto r2 = Revision::thread_revision()->fork(
		[&stack]() { ASSERT_EQ(stack.size(), 0); });

	ASSERT_EQ(stack.size(), 0);

	Revision::thread_revision()->join(r2);
	ASSERT_EQ(stack.size(), 0);

	Revision::thread_revision()->join(r1);
	ASSERT_EQ(stack.size(), 0);
}

TEST(VStackTest, long_segment_chain)
{
	VStack<int> stack;

	std::shared_ptr<Revision> r1, r2, r3, r4;

	// 1st thread
	r1 = Revision::thread_revision()->fork(
		[&stack]() { ASSERT_EQ(stack.size(), 0); });

	// 2nd thread
	r2 = Revision::thread_revision()->fork(
		[&stack]() { ASSERT_EQ(stack.size(), 0); });
	stack.push(1);

	// 3rd thread
	r3 = Revision::thread_revision()->fork(
		[&stack]() { ASSERT_EQ(stack.size(), 1); });

	// 4th thread
	r4 = Revision::thread_revision()->fork(
		[&stack]() { ASSERT_EQ(stack.size(), 1); });

	ASSERT_EQ(stack.size(), 1);

	Revision::thread_revision()->join(r1);
	ASSERT_EQ(stack.size(), 1);

	Revision::thread_revision()->join(r2);
	ASSERT_EQ(stack.size(), 1);

	Revision::thread_revision()->join(r3);
	ASSERT_EQ(stack.size(), 1);

	Revision::thread_revision()->join(r4);
	ASSERT_EQ(stack.top(), 1);
	ASSERT_EQ(stack.size(), 1);
}

TEST(VStackTest, multithread_user_strategy)
{
	VStack<int> stack(
		[](const std::stack<int> &was, const std::stack<int> &main,
		   const std::stack<int> &current) { return current; });

	for (int i = 0; i < 10; i++) {
		stack.push(i);
	}
	ASSERT_EQ(stack.size(), 10);

	volatile bool synchro = false;

	// 1st thread
	const auto r1 = Revision::thread_revision()->fork([&stack, &synchro]() {
		while (!synchro) {
		} // wait until 2nd thread set var

		ASSERT_EQ(stack.size(), 10);

		for (int i = 0; i < 5; i++) {
			ASSERT_EQ(stack.top(), 10 - i - 1);
			stack.pop();
		}
		ASSERT_EQ(stack.size(), 5);

		for (int i = 0; i < 5; i++) {
			stack.push(i * 3); // top: 12 9 6 3 0 4 3 2 1 0
		}
		ASSERT_EQ(stack.size(), 10);
	});

	// 2nd thread
	const auto r2 = Revision::thread_revision()->fork([&stack]() {
		ASSERT_EQ(stack.size(), 10);

		for (int i = 0; i < 3; i++) {
			ASSERT_EQ(stack.top(), 10 - i - 1);
			stack.pop();
		}
		ASSERT_EQ(stack.size(), 7);

		for (int i = 0; i < 7; i++) {
			stack.push(i *
				   10); // top:60 50 40 30 20 10 0 6 5 4 3 2 1 0
		}
		ASSERT_EQ(stack.size(), 14);
	});

	ASSERT_EQ(stack.size(), 10);

	for (int i = 0; i < 8; i++) {
		ASSERT_EQ(stack.top(), 10 - i - 1);
		stack.pop();
	}
	ASSERT_EQ(stack.size(), 2);

	/*
	 * 1. r2:		60 50 40 30 20 10 0 6 5 4 3 2 1 0
	 * 2. main:		      		      	      1 0
	 * 3. original stack:   	      9 8 7 6 5 4 3 2 1 0
	 * 3. res:      	60 50 40 30 20 10 0 6 5 4 3 2 1 0
	 */

	// let first thread stop later
	Revision::thread_revision()->join(r2);
	ASSERT_EQ(stack.size(), 14);

	/*
	 * 1. r1:			     12 9 6 3 0 4 3 2 1 0
	 * 2. main:		60 50 40 30 20 10 0 6 5 4 3 2 1 0
	 * 3. original stack:   	      9 8 7 6 5 4 3 2 1 0
	 * 3. res:             		     12 9 6 3 0 4 3 2 1 0
	 */

	synchro = true;
	Revision::thread_revision()->join(r1);
	ASSERT_EQ(stack.size(), 10);

	std::vector<int> good_answer = { 0, 1, 2, 3, 4, 0, 3, 6, 9, 12 };
	while (stack.size()) {
		ASSERT_EQ(stack.top(), good_answer.back());
		stack.pop();
		good_answer.pop_back();
	}
}

TEST(VStackTest, big_test)
{
	int number_of_added = 30;
	int threads_amount = 20;
	int main_length = 50;

	VStack<int> stack;
	std::stack<int> result_stack;
	std::vector<std::shared_ptr<Revision> > threads(threads_amount);

	for (int i = 0; i < main_length; ++i) {
		stack.push(i);
	}

	ASSERT_EQ(stack.size(), main_length);

	for (auto &thread : threads) {
		thread = Revision::thread_revision()->fork([&stack,
							    &number_of_added,
							    &main_length]() {
			ASSERT_EQ(stack.size(), main_length);

			std::this_thread::sleep_for(std::chrono::milliseconds(
				(rand() % 5 + 1) * 1000));

			for (int j = 0; j < number_of_added; j++) {
				stack.push(j * 13);
				std::this_thread::sleep_for(
					std::chrono::milliseconds(
						(rand() % 5 + 1) * 100));
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(
				(rand() % 5 + 1) * 100));

			ASSERT_EQ(stack.size(), main_length + number_of_added);

		});
		std::this_thread::sleep_for(
			std::chrono::milliseconds((rand() % 9 + 1) * 100));
	}

	for (const auto &thread : threads) {
		Revision::thread_revision()->join(thread);
	}

	ASSERT_EQ(stack.size(), main_length + threads_amount * number_of_added);

	for (int i = 0; i < main_length; ++i) {
		result_stack.push(i);
	}

	ASSERT_EQ(result_stack.size(), main_length);

	for (int i = 0; i < threads_amount; ++i) {
		for (int j = 0; j < number_of_added; ++j) {
			result_stack.push(j * 13);
		}
	}

	ASSERT_EQ(stack.size(), result_stack.size());

	while (!stack.empty()) {
		ASSERT_EQ(stack.top(), result_stack.top());

		stack.pop();
		result_stack.pop();
	}
	ASSERT_TRUE(stack.empty());
}