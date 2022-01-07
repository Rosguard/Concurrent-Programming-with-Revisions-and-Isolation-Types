#include "VBasicType.h"
#include <gtest/gtest.h>

TEST(VBasicTypeTest, set_get)
{
	VBasicType<int> var;

	var.set(1);
	ASSERT_EQ(var.get(), 1);
}

TEST(VBasicTypeTest, last_set)
{
	VBasicType<int> var;

	var.set(1);
	var.set(2);
	ASSERT_EQ(var.get(), 2);
}

TEST(VBasicTypeTest, multithread)
{
	VBasicType<int> var;

	var.set(1);
	volatile bool synchro = false;

	// 1st thread
	const auto r1 = Revision::thread_revision()->fork([&var, &synchro]() {
		while (!synchro) {
		} // wait until 2nd thread set var

		ASSERT_EQ(var.get(), 1);

		var.set(2);
		ASSERT_EQ(var.get(), 2);
	});

	// 2nd thread
	const auto r2 = Revision::thread_revision()->fork([&var]() {
		ASSERT_EQ(var.get(), 1);

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

TEST(VBasicTypeTest, perfect_nested_threads)
{
	VBasicType<int> var;

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

TEST(VBasicTypeTest, nested_threads)
{
	VBasicType<int> var;

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

TEST(VBasicTypeTest, multithread_user_strategy)
{
	VBasicType<int> var([](const int &was, const int &main,
			       const int &current) { return main + current; });

	var.set(1);
	volatile bool synchro = false;

	// 1st thread
	const auto r1 = Revision::thread_revision()->fork([&var, &synchro]() {
		while (!synchro) {
		} // wait until 2nd thread set var

		ASSERT_EQ(var.get(), 1);

		var.set(2);
		ASSERT_EQ(var.get(), 2);
	});

	// 2nd thread
	const auto r2 = Revision::thread_revision()->fork([&var]() {
		ASSERT_EQ(var.get(), 1);

		var.set(3);
		ASSERT_EQ(var.get(), 3);
	});

	ASSERT_EQ(var.get(), 1);

	// let first thread stop later
	Revision::thread_revision()->join(r2);
	ASSERT_EQ(var.get(), 4);

	synchro = true;
	Revision::thread_revision()->join(r1);
	ASSERT_EQ(var.get(), 6);
}
