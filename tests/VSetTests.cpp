#include "VSet.h"
#include <gtest/gtest.h>

TEST(VSetTest, instert_find_iterators)
{
	VSet<int> set;

	set.insert(1);

	ASSERT_NE(set.find(1), set.end());
	ASSERT_EQ(set.find(1), set.begin());
	ASSERT_EQ(set.find(2), set.end());
}

TEST(VSetTest, size_empty_clear_erase)
{
	VSet<int> set;

	ASSERT_TRUE(set.empty());
	ASSERT_EQ(set.size(), 0);

	set.insert(1);
	ASSERT_FALSE(set.empty());

	set.insert(2);
	ASSERT_EQ(set.size(), 2);

	set.erase(1);
	ASSERT_EQ(set.size(), 1);
	ASSERT_EQ(set.find(2), set.begin());
	ASSERT_FALSE(set.empty());

	set.clear();
	ASSERT_EQ(set.size(), 0);
	ASSERT_TRUE(set.empty());
}

TEST(VSetTest, count)
{
	VSet<int> set;

	set.insert(1);
	set.insert(1);
	set.insert(1);
	set.insert(2);

	ASSERT_EQ(set.count(1), 1); // dont allow duplicates
	ASSERT_EQ(set.count(2), 1);
}

TEST(VSetTest, equal_range)
{
	VSet<int> set;

	set.insert(1);
	set.insert(2);
	set.insert(2);
	set.insert(2);
	set.insert(3);

	const auto range = set.equal_range(2);
	auto range_iter = set.begin();
	range_iter++;
	ASSERT_EQ(range.first, range_iter);
	range_iter++;
	range_iter++;
	range_iter++;
	ASSERT_EQ(range.second, range_iter);
}

TEST(VSetTest, bounds)
{
	VSet<int> set;

	set.insert(1);
	set.insert(3);
	set.insert(4);
	set.insert(5);
	set.insert(7);

	auto range_iter = set.begin();

	range_iter++;
	ASSERT_EQ(set.lower_bound(2), range_iter);

	range_iter++;
	range_iter++;
	range_iter++;
	ASSERT_EQ(set.upper_bound(6), range_iter);
}

TEST(VSetTest, swap)
{
	VSet<int> set1, set2;

	set1.insert(1);
	set1.insert(2);
	ASSERT_EQ(set1.find(3), set1.end());

	set2.insert(3);
	set2.insert(4);
	ASSERT_EQ(set2.find(1), set2.end());

	set1.swap(set2);

	ASSERT_EQ(set2.find(4), set2.end());
	ASSERT_NE(set1.find(3), set1.end());
	ASSERT_EQ(set1.find(2), set1.end());
	ASSERT_NE(set2.find(1), set2.end());
}

TEST(VSetTest, multithread_test)
{
	VSet<int> set;
	VSet<int> set2;

	for (int i = 0; i < 10; i++) {
		set.insert(i);
		set2.insert(i);
	}
	ASSERT_EQ(set.size(), 10);
	ASSERT_EQ(set2.size(), 10);

	for (int i = 5; i < 10; i++) {
		set2.erase(i);
	}

	ASSERT_EQ(set2.size(), 5);

	volatile bool synchro = false;

	// 1st thread
	const auto r1 =
		Revision::thread_revision()->fork([&set, &set2, &synchro]() {
			while (!synchro) {
			} // wait until 2nd thread set var

			ASSERT_EQ(set.size(), 10);
			ASSERT_EQ(set2.size(), 5);

			set.swap(set2);

			ASSERT_EQ(set.size(), 5);
			ASSERT_EQ(set2.size(), 10);

			for (int i = 0; i < 5; i++) {
				set.insert(i * 3); // 12 9 6 4 3 2 1 0
			}
			ASSERT_EQ(set.size(), 8);
		});

	// 2nd thread
	const auto r2 = Revision::thread_revision()->fork([&set]() {
		ASSERT_EQ(set.size(), 10);

		for (int i = 0; i < 3; i++) {
			ASSERT_NE(set.find(10 - i - 1), set.end());
			set.erase(10 - i - 1);
		}
		ASSERT_EQ(set.size(), 7);

		for (int i = 0; i < 7; i++) {
			set.insert(i *
				   10); // top:60 50 40 30 20 10 6 5 4 3 2 1 0
		}
		ASSERT_EQ(set.size(), 13);
	});

	ASSERT_EQ(set.size(), 10);

	for (int i = 0; i < 8; i++) {
		ASSERT_NE(set.find(10 - i - 1), set.end());
		set.erase(10 - i - 1);
	}
	ASSERT_EQ(set.size(), 2);

	/*
	 * 1. r2:	        60 50 40 30 20 10 6 5 4 3 2 1 0
	 * 2. main:		      		      	    1 0
	 * 3. original set:   	            9 8 7 6 5 4 3 2 1 0
	 * 4. res:      		  60 50 40 30 20 10 1 0
	*/

	// let first thread stop later
	Revision::thread_revision()->join(r2);
	ASSERT_EQ(set.size(), 8);

	/*
	 * 1. r1:			         12 9 6 4 3 2 1 0
	 * 2. main:		            60 50 40 30 20 10 1 0
	 * 3. original stack:   	      9 8 7 6 5 4 3 2 1 0
	 * 3. res:                       12 60 50 40 30 20 10 1 0
	 */

	synchro = true;
	Revision::thread_revision()->join(r1);
	ASSERT_EQ(set.size(), 9);

	std::vector<int> good_answer = { 12, 60, 50, 40, 30, 20, 10, 1, 0 };
	std::for_each(good_answer.begin(), good_answer.end(),
		      [&set](const int &n) {
			      ASSERT_NE(set.find(n), set.end());
			      set.erase(n);
		      });
	ASSERT_TRUE(set.empty());
}
