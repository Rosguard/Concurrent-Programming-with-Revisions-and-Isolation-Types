#include "VDataStructure.h"
#include <gtest/gtest.h>

TEST(VDataStructureTest, set_get)
{
	VDataStructure<int> var;

	var.set(1);

	ASSERT_EQ(var.get(), 1);
}

TEST(VDataStructureTest, last_set)
{
	VDataStructure<int> var;

	var.set(1);
	var.set(2);

	ASSERT_EQ(var.get(), 2);
}

TEST(VDataStructureTest, basic_multithread_test)
{
	VDataStructure<int> var;

	var.set(1);
	volatile bool synchro = false;

	// 1st thread
	const auto r1 = Revision::thread_revision()->fork([&var, &synchro]() {
		while (!synchro) {
		} // wait until 2nd thread set var

		ASSERT_EQ(var.get(), 1);

		DEBUG_ONLY(std::string(test_info_->name()) +
			   " set by thread 1");
		var.set(2);
		ASSERT_EQ(var.get(), 2);
	});

	// 2nd thread
	const auto r2 = Revision::thread_revision()->fork([&var]() {
		ASSERT_EQ(var.get(), 1);

		DEBUG_ONLY(std::string(test_info_->name()) +
			   " set by thread 2");
		var.set(3);
		ASSERT_EQ(var.get(), 3);
	});

	ASSERT_EQ(var.get(), 1);

	// let first thread stop later
	Revision::thread_revision()->join(r2);
	ASSERT_EQ(var.get(), 3);

	synchro = true;
	Revision::thread_revision()->join(r1);

	ASSERT_EQ(var.get(), 2);
}

TEST(VDataStructureTest, perfect_nested_threads)
{
	VDataStructure<int> var;

	var.set(1);

	// 1st thread
	const auto r1 = Revision::thread_revision()->fork([&var]() {
		ASSERT_EQ(var.get(), 1);

		// 2nd thread
		const auto r2 = Revision::thread_revision()->fork([&var]() {
			ASSERT_EQ(var.get(), 1);

			var.set(3);
			ASSERT_EQ(var.get(), 3);
		});

		var.set(2);

		Revision::thread_revision()->join(r2);
		ASSERT_EQ(var.get(), 3);
	});

	ASSERT_EQ(var.get(), 1);

	Revision::thread_revision()->join(r1);
	ASSERT_EQ(var.get(), 3);
}

TEST(VDataStructureTest, nested_threads)
{
	VDataStructure<int> var;

	var.set(1);
	std::shared_ptr<Revision> r2;

	// 1st thread
	const auto r1 = Revision::thread_revision()->fork([&var, &r2]() {
		ASSERT_EQ(var.get(), 1);

		// 2nd thread
		r2 = Revision::thread_revision()->fork([&var]() {
			ASSERT_EQ(var.get(), 1);

			var.set(3);
			ASSERT_EQ(var.get(), 3);
		});

		ASSERT_EQ(var.get(), 1);
		var.set(2);
	});

	ASSERT_EQ(var.get(), 1);

	Revision::thread_revision()->join(r1);
	ASSERT_EQ(var.get(), 2);
	Revision::thread_revision()->join(r2);
	ASSERT_EQ(var.get(), 3);
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
