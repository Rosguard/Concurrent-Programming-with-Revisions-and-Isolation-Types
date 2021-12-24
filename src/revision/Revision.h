#pragma once

#include "segment/Segment.h"
#include <memory>
#include <thread>
#include <functional>
#include <utility>

class Revision : public std::enable_shared_from_this<Revision> {
    private:
	std::shared_ptr<Segment> _root;
	std::shared_ptr<Segment> _current;

	std::thread _task;

	thread_local static std::shared_ptr<Revision> _current_revision;

    public:
	Revision(std::shared_ptr<Segment> root,
		 std::shared_ptr<Segment> current)
		: _root(std::move(root)), _current(std::move(current))
	{
	}

	std::shared_ptr<Revision> fork(const std::function<void()> &action);
	void join(const std::shared_ptr<Revision> &other_revision);

	// getters
	inline std::shared_ptr<Segment> current() const
	{
		return _current;
	}

	inline static std::shared_ptr<Revision> thread_revision()
	{
		return _current_revision;
	}

	inline std::shared_ptr<Segment> root() const
	{
		return _root;
	}
};
