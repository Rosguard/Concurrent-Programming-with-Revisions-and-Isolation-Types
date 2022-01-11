#include "VQueue.h"
#include <gtest/gtest.h>

TEST(VQueueTest, push_back)
{
	VQueue<int> queue;

	queue.push(1);
	ASSERT_EQ(queue.back(), 1);

	queue.push(3);
	ASSERT_EQ(queue.back(), 3);
}

TEST(VQueueTest, push_front)
{
	VQueue<int> queue;

	queue.push(1);
	ASSERT_EQ(queue.front(), 1);

	queue.push(4);
	ASSERT_EQ(queue.front(), 1);
}

TEST(VQueueTest, push_pop_size)
{
	VQueue<int> queue;

	queue.push(1);
	ASSERT_EQ(queue.size(), 1);

	queue.pop();
	ASSERT_EQ(queue.size(), 0);
}

TEST(VQueueTest, empty_test)
{
	VQueue<int> queue;
	ASSERT_EQ(queue.size(), 0);
	ASSERT_EQ(queue.empty(), true);

	queue.push(1);
	ASSERT_EQ(queue.size(), 1);
	ASSERT_EQ(queue.empty(), false);

	queue.pop();
	ASSERT_EQ(queue.size(), 0);
	ASSERT_EQ(queue.empty(), true);
}

TEST(VQueueTest, swap_test)
{
	VQueue<int> first_queue;
	ASSERT_EQ(first_queue.size(), 0);

	VQueue<int> second_queue;
	ASSERT_EQ(second_queue.size(), 0);

	for (int i = 0; i < 6; ++i) {
		first_queue.push(i * 30);
	}
	ASSERT_EQ(first_queue.size(), 6);

	for (int i = 0; i < 10; ++i) {
		second_queue.push(i * 100);
	}
	ASSERT_EQ(second_queue.size(), 10);

	first_queue.swap(second_queue);

	ASSERT_EQ(second_queue.size(), 6);
	ASSERT_EQ(first_queue.size(), 10);
}

TEST(VQueueTest, multithread_1)
{
	VQueue<int> queue;

	for (int i = 0; i < 10; i++) {
		queue.push(i); //front: 0 1 2 3 4 5 6 7 8 9
	}
	ASSERT_EQ(queue.size(), 10);

	volatile bool synchro = false;

	// 1st thread
	const auto r1 = Revision::thread_revision()->fork([&queue, &synchro]() {
		while (!synchro) {
		} // wait until 2nd thread set var

		ASSERT_EQ(queue.size(), 10);

		for (int i = 0; i < 5; i++) {
			ASSERT_EQ(queue.front(), i);
			queue.pop(); //front: 5 6 7 8 9
		}
		ASSERT_EQ(queue.size(), 5);

		for (int i = 0; i < 5; i++) {
			queue.push(i * 3); //front: 5 6 7 8 9 0 3 6 9 12
		}
		ASSERT_EQ(queue.size(), 10);
	});

	// 2nd thread
	const auto r2 = Revision::thread_revision()->fork([&queue]() {
		ASSERT_EQ(queue.size(), 10);

		for (int i = 0; i < 3; i++) {
			ASSERT_EQ(queue.front(), i);
			queue.pop(); //front: 3 4 5 6 7 8 9
		}
		ASSERT_EQ(queue.size(), 7);

		for (int i = 0; i < 7; i++) {
			queue.push(
				i *
				10); //front: 3 4 5 6 7 8 9 0 10 20 30 40 50 60
		}
		ASSERT_EQ(queue.size(), 14);
	});

	ASSERT_EQ(queue.size(), 10);

	for (int i = 0; i < 8; i++) {
		ASSERT_EQ(queue.front(), i);
		queue.pop(); //front: 8 9
	}
	ASSERT_EQ(queue.size(), 2);

	/*
	 * 1. r2:		3 4 5 6 7 8 9 0 10 20 30 40 50 60
	 * 2. main:		      		      	      8 9
	 * 3. original queue:   	      0 1 2 3 4 5 6 7 8 9
	 * 3. res:		          8 9 0 10 20 30 40 50 60
	 */

	// let first thread stop later
	Revision::thread_revision()->join(r2);
	ASSERT_EQ(queue.size(), 9);
	/*
	 * 1. r1:			     5 6 7 8 9 0 3 6 9 12
	 * 2. main:		          8 9 0 10 20 30 40 50 60
	 * 3. original queue:   	      0 1 2 3 4 5 6 7 8 9
	 * 3. res:             8 9 0 10 20 30 40 50 60 0 3 6 9 12
	 */

	synchro = true;
	Revision::thread_revision()->join(r1);
	ASSERT_EQ(queue.size(), 14);

	std::vector<int> good_answer = { 12, 9,	 6,  3,	 0, 60, 50,
					 40, 30, 20, 10, 0, 9,	8 };
	while (queue.size()) {
		ASSERT_EQ(queue.front(), good_answer.back());
		queue.pop();
		good_answer.pop_back();
	}
}

TEST(VQueueTest, multithread_2)
{
	VQueue<int> queue;

	for (int i = 0; i < 10; i++) {
		queue.push(i); //front: 0 1 2 3 4 5 6 7 8 9
	}
	ASSERT_EQ(queue.size(), 10);

	volatile bool synchro = false;

	// 1st thread
	const auto r1 = Revision::thread_revision()->fork([&queue, &synchro]() {
		while (!synchro) {
		} // wait until 2nd thread set var

		ASSERT_EQ(queue.size(), 10);

		for (int i = 1; i < 6; i++) {
			queue.push(i * 100);
			//front: 0 1 2 3 4 5 6 7 8 9 100 200 300 400 500
		}
		ASSERT_EQ(queue.size(), 15);

		for (int i = 0; i < 10; i++) {
			ASSERT_EQ(queue.front(), i);
			queue.pop(); //front: 100 200 300 400 500
		}
		ASSERT_EQ(queue.size(), 5);
	});

	// 2nd thread
	const auto r2 = Revision::thread_revision()->fork([&queue]() {
		ASSERT_EQ(queue.size(), 10);

		for (int i = 0; i < 3; i++) {
			ASSERT_EQ(queue.front(), i);
			queue.pop(); //front: 3 4 5 6 7 8 9
		}
		ASSERT_EQ(queue.size(), 7);

		for (int i = 0; i < 7; i++) {
			queue.push(
				i *
				10); //front: 3 4 5 6 7 8 9 0 10 20 30 40 50 60
		}
		ASSERT_EQ(queue.size(), 14);
	});

	ASSERT_EQ(queue.size(), 10);

	for (int i = 1; i <= 3; i++) {
		queue.push(i * 30); //front: 0 1 2 3 4 5 6 7 8 9 30 60 90
	}
	ASSERT_EQ(queue.size(), 13);

	for (int i = 0; i < 11; i++) {
		queue.pop(); //front: 60 90
	}
	ASSERT_EQ(queue.size(), 2);

	/*
	 * 1. r2:		3 4 5 6 7 8 9 0 10 20 30 40 50 60
	 * 2. main:		      		     	    60 90
	 * 3. _original_queue:   	      0 1 2 3 4 5 6 7 8 9
	 * 3. res:		 	60 90 0 10 20 30 40 50 60
	 */

	// let first thread stop later
	Revision::thread_revision()->join(r2);
	ASSERT_EQ(queue.size(), 9);

	/*
	 * 1. r1:			      100 200 300 400 500
	 * 2. main:		 	60 90 0 10 20 30 40 50 60
	 * 3. _original_queue:   	      0 1 2 3 4 5 6 7 8 9
	 * 3. res:  60 90 0 10 20 30 40 50 60 100 200 300 400 500
	 */

	synchro = true;
	Revision::thread_revision()->join(r1);
	ASSERT_EQ(queue.size(), 14);

	std::vector<int> good_answer = { 500, 400, 300, 200, 100, 60, 50,
					 40,  30,  20,	10,  0,	  90, 60 };
	while (queue.size()) {
		ASSERT_EQ(queue.front(), good_answer.back());
		queue.pop();
		good_answer.pop_back();
	}
}

TEST(VQueueTest, multithread_3)
{
	VQueue<int> queue;

	for (int i = 0; i < 10; i++) {
		queue.push(i); //front: 0 1 2 3 4 5 6 7 8 9
	}
	ASSERT_EQ(queue.size(), 10);

	volatile bool synchro = false;

	// 1st thread
	const auto r1 = Revision::thread_revision()->fork([&queue, &synchro]() {
		while (!synchro) {
		} // wait until 2nd thread set var

		ASSERT_EQ(queue.size(), 10);

		for (int i = 1; i < 6; i++) {
			queue.push(i * 100);
			//front: 0 1 2 3 4 5 6 7 8 9 100 200 300 400 500
		}
		ASSERT_EQ(queue.size(), 15);

		for (int i = 0; i < 6; i++) {
			ASSERT_EQ(queue.front(), i);
			queue.pop(); //front: 6 7 8 9 100 200 300 400 500
		}
		ASSERT_EQ(queue.size(), 9);
	});

	// 2nd thread
	const auto r2 = Revision::thread_revision()->fork([&queue]() {
		ASSERT_EQ(queue.size(), 10);

		for (int i = 1; i < 3; i++) {
			queue.push(i * 10); //front: 0 1 2 3 4 5 6 7 8 9 10 20
		}
		ASSERT_EQ(queue.size(), 12);
	});

	ASSERT_EQ(queue.size(), 10);

	for (int i = 1; i <= 3; i++) {
		queue.push(i * 30); //front: 0 1 2 3 4 5 6 7 8 9 30 60 90
	}
	ASSERT_EQ(queue.size(), 13);

	for (int i = 0; i < 3; i++) {
		ASSERT_EQ(queue.front(), i);
		queue.pop(); //front: 3 4 5 6 7 8 9 30 60 90
	}
	ASSERT_EQ(queue.size(), 10);

	/*
	 * 1. r2:			0 1 2 3 4 5 6 7 8 9 10 20
	 * 2. main:		    	   3 4 5 6 7 8 9 30 60 90
	 * 3. _original_queue:   	      0 1 2 3 4 5 6 7 8 9
	 * 3. res:		     3 4 5 6 7 8 9 30 60 90 10 20
	 */

	// let first thread stop later
	Revision::thread_revision()->join(r2);

	ASSERT_EQ(queue.size(), 12);

	std::vector<int> good_res1 = { 20, 10, 90, 60, 30, 9, 8, 7, 6, 5, 4, 3 };
	VQueue<int> queue_copy = queue;

	while (queue_copy.size()) {
		ASSERT_EQ(queue_copy.front(), good_res1.back());
		queue_copy.pop();
		good_res1.pop_back();
	}

	/*
	 * 1. r1:		      6 7 8 9 100 200 300 400 500
	 * 2. main:		     3 4 5 6 7 8 9 30 60 90 10 20
	 * 3. _original_queue:   	      0 1 2 3 4 5 6 7 8 9
	 * 3. res:     6 7 8 9 30 60 90 10 20 100 200 300 400 500
	 */

	synchro = true;
	Revision::thread_revision()->join(r1);

	ASSERT_EQ(queue.size(), 14);

	std::vector<int> good_answer = { 500, 400, 300, 200, 100, 20, 10,
					 90,  60,  30,	9,   8,	  7,  6 };
	while (queue.size()) {
		ASSERT_EQ(queue.front(), good_answer.back());
		queue.pop();
		good_answer.pop_back();
	}
}

TEST(VQueueTest, nothing_doing_test)
{
	VQueue<int> queue;
	ASSERT_EQ(queue.size(), 0);

	// 1st thread
	const auto r1 = Revision::thread_revision()->fork(
		[&queue]() { ASSERT_EQ(queue.size(), 0); });

	// 2nd thread
	const auto r2 = Revision::thread_revision()->fork(
		[&queue]() { ASSERT_EQ(queue.size(), 0); });

	ASSERT_EQ(queue.size(), 0);

	Revision::thread_revision()->join(r2);
	ASSERT_EQ(queue.size(), 0);

	Revision::thread_revision()->join(r1);
	ASSERT_EQ(queue.size(), 0);
}

TEST(VQueueTest, long_segment_chain)
{
	VQueue<int> queue;

	std::shared_ptr<Revision> r1, r2, r3, r4;

	// 1st thread
	r1 = Revision::thread_revision()->fork(
		[&queue]() { ASSERT_EQ(queue.size(), 0); });

	// 2nd thread
	r2 = Revision::thread_revision()->fork(
		[&queue]() { ASSERT_EQ(queue.size(), 0); });
	queue.push(1);

	// 3rd thread
	r3 = Revision::thread_revision()->fork(
		[&queue]() { ASSERT_EQ(queue.size(), 1); });

	// 4th thread
	r4 = Revision::thread_revision()->fork(
		[&queue]() { ASSERT_EQ(queue.size(), 1); });

	ASSERT_EQ(queue.size(), 1);

	Revision::thread_revision()->join(r1);
	ASSERT_EQ(queue.size(), 1);

	Revision::thread_revision()->join(r2);
	ASSERT_EQ(queue.size(), 1);

	Revision::thread_revision()->join(r3);
	ASSERT_EQ(queue.size(), 1);

	Revision::thread_revision()->join(r4);
	ASSERT_EQ(queue.front(), 1);
	ASSERT_EQ(queue.size(), 1);
}

TEST(VQueueTest, multithread_user_stategy)
{
	VQueue<int> queue([](const std::queue<int> &was,
			     const std::queue<int> &main,
			     const std::queue<int> &current) { return main; });

	for (int i = 0; i < 10; i++) {
		queue.push(i); //front: 0 1 2 3 4 5 6 7 8 9
	}
	ASSERT_EQ(queue.size(), 10);

	volatile bool synchro = false;

	// 1st thread
	const auto r1 = Revision::thread_revision()->fork([&queue, &synchro]() {
		while (!synchro) {
		} // wait until 2nd thread set var

		ASSERT_EQ(queue.size(), 10);

		for (int i = 0; i < 5; i++) {
			ASSERT_EQ(queue.front(), i);
			queue.pop(); //front: 5 6 7 8 9
		}
		ASSERT_EQ(queue.size(), 5);

		for (int i = 0; i < 5; i++) {
			queue.push(i * 3); //front: 5 6 7 8 9 0 3 6 9 12
		}
		ASSERT_EQ(queue.size(), 10);
	});

	// 2nd thread
	const auto r2 = Revision::thread_revision()->fork([&queue]() {
		ASSERT_EQ(queue.size(), 10);

		for (int i = 0; i < 3; i++) {
			ASSERT_EQ(queue.front(), i);
			queue.pop(); //front: 3 4 5 6 7 8 9
		}
		ASSERT_EQ(queue.size(), 7);

		for (int i = 0; i < 7; i++) {
			queue.push(
				i *
				10); //front: 3 4 5 6 7 8 9 0 10 20 30 40 50 60
		}
		ASSERT_EQ(queue.size(), 14);
	});

	ASSERT_EQ(queue.size(), 10);

	for (int i = 0; i < 8; i++) {
		ASSERT_EQ(queue.front(), i);
		queue.pop(); //front: 8 9
	}
	ASSERT_EQ(queue.size(), 2);

	/*
	 * 1. r2:		3 4 5 6 7 8 9 0 10 20 30 40 50 60
	 * 2. main:		      		      	      8 9
	 * 3. original queue:   	      0 1 2 3 4 5 6 7 8 9
	 * 3. res:		          		      8 9
	 */

	// let first thread stop later
	Revision::thread_revision()->join(r2);
	ASSERT_EQ(queue.size(), 2);
	/*
	 * 1. r1:			     5 6 7 8 9 0 3 6 9 12
	 * 2. main:		          		      8 9
	 * 3. original queue:   	      0 1 2 3 4 5 6 7 8 9
	 * 3. res:             				      8 9
	 */

	synchro = true;
	Revision::thread_revision()->join(r1);
	ASSERT_EQ(queue.size(), 2);

	std::vector<int> good_answer = { 9, 8 };
	while (queue.size()) {
		ASSERT_EQ(queue.front(), good_answer.back());
		queue.pop();
		good_answer.pop_back();
	}
}

TEST(VQueueTest, big_test)
{
	int number_of_added = 30;
	int threads_amount = 20;
	int main_length = 50;

	VQueue<int> queue;
	std::queue<int> result_queue;
	std::vector<std::shared_ptr<Revision> > threads(threads_amount);

	for (int i = 0; i < main_length; ++i) {
		queue.push(i);
	}

	ASSERT_EQ(queue.size(), main_length);

	for (auto &thread : threads) {
		thread = Revision::thread_revision()->fork([&queue,
							    &number_of_added,
							    &main_length]() {
			ASSERT_EQ(queue.size(), main_length);

			std::this_thread::sleep_for(std::chrono::milliseconds(
				(rand() % 5 + 1) * 1000));

			for (int j = 0; j < number_of_added; j++) {
				queue.push(j * 13);
				std::this_thread::sleep_for(
					std::chrono::milliseconds(
						(rand() % 5 + 1) * 100));
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(
				(rand() % 5 + 1) * 100));

			ASSERT_EQ(queue.size(), main_length + number_of_added);

		});
		std::this_thread::sleep_for(
			std::chrono::milliseconds((rand() % 9 + 1) * 100));
	}

	for (const auto &thread : threads) {
		Revision::thread_revision()->join(thread);
	}

	ASSERT_EQ(queue.size(), main_length + threads_amount * number_of_added);

	for (int i = 0; i < main_length; ++i) {
		result_queue.push(i);
	}

	ASSERT_EQ(result_queue.size(), main_length);

	for (int i = 0; i < threads_amount; ++i) {
		for (int j = 0; j < number_of_added; ++j) {
			result_queue.push(j * 13);
		}
	}

	ASSERT_EQ(queue.size(), result_queue.size());

	while (!queue.empty()) {
		ASSERT_EQ(queue.front(), result_queue.front());

		queue.pop();
		result_queue.pop();
	}
	ASSERT_TRUE(queue.empty());
}