#include "VQueue.h"
#include <gtest/gtest.h>

TEST(VQueueTest, push_back)
{
	DEBUG_ONLY("\nSTART TEST " + std::string(test_info_->name()));

	VQueue<int> queue;

	queue.push(1);
	ASSERT_EQ(queue.back(), 1);

	queue.push(3);
	ASSERT_EQ(queue.back(), 3);
}

TEST(VQueueTest, push_front)
{
	DEBUG_ONLY("\nSTART TEST " + std::string(test_info_->name()));

	VQueue<int> queue;

	queue.push(1);
	ASSERT_EQ(queue.front(), 1);

	queue.push(4);
	ASSERT_EQ(queue.front(), 1);
}

TEST(VQueueTest, push_pop_size)
{
	DEBUG_ONLY("\nSTART TEST " + std::string(test_info_->name()));

	VQueue<int> queue;

	queue.push(1);
	ASSERT_EQ(queue.size(), 1);

	queue.pop();
	ASSERT_EQ(queue.size(), 0);
}

TEST(VQueueTest, basic_multithread_test_1)
{
	DEBUG_ONLY("\nSTART TEST " + std::string(test_info_->name()));

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

		DEBUG_ONLY(std::string(test_info_->name()) +
			   " set by thread 1");

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

		DEBUG_ONLY(std::string(test_info_->name()) +
			   " set by thread 2");

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
	 * 3. _original_queue:   	      0 1 2 3 4 5 6 7 8 9
	 * 3. res:		          8 9 0 10 20 30 40 50 60
	 */

	// let first thread stop later
	Revision::thread_revision()->join(r2);
	ASSERT_EQ(queue.size(), 9);
	/*
	 * 1. r1:			     5 6 7 8 9 0 3 6 9 12
	 * 2. main:		          8 9 0 10 20 30 40 50 60
	 * 3. _original_queue:   	      0 1 2 3 4 5 6 7 8 9
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

TEST(VQueueTest, basic_multithread_test_2)
{
	DEBUG_ONLY("\nSTART TEST " + std::string(test_info_->name()));

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

		DEBUG_ONLY(std::string(test_info_->name()) +
			   " set by thread 1");
	});

	// 2nd thread
	const auto r2 = Revision::thread_revision()->fork([&queue]() {
		ASSERT_EQ(queue.size(), 10);

		for (int i = 0; i < 3; i++) {
			ASSERT_EQ(queue.front(), i);
			queue.pop(); //front: 3 4 5 6 7 8 9
		}
		ASSERT_EQ(queue.size(), 7);

		DEBUG_ONLY(std::string(test_info_->name()) +
			   " set by thread 2");

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

TEST(VQueueTest, basic_multithread_test_3)
{
	DEBUG_ONLY("\nSTART TEST " + std::string(test_info_->name()));

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

		DEBUG_ONLY(std::string(test_info_->name()) +
			   " set by thread 1");
	});

	// 2nd thread
	const auto r2 = Revision::thread_revision()->fork([&queue]() {
		ASSERT_EQ(queue.size(), 10);

		DEBUG_ONLY(std::string(test_info_->name()) +
			   " set by thread 2");

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
