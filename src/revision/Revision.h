#pragma once

#include "segment/Segment.h"
#include <memory>
#include <thread>
#include <functional>
#include <utility>

class Revision {
    private:
	std::shared_ptr<Segment> _root;
	std::shared_ptr<Segment> _current;

	std::thread _task;

	thread_local static std::shared_ptr<Revision> _current_revision;

    public:
	Revision(const std::shared_ptr<Segment> &root,
		 const std::shared_ptr<Segment> &current);

	std::shared_ptr<Revision> fork(const std::function<void()> &action);
	void join(const std::shared_ptr<Revision> &other_revision);

	// getters
	[[nodiscard]] inline std::shared_ptr<Segment> current() const
	{
		return _current;
	}

	[[nodiscard]] inline std::shared_ptr<Segment> root() const
	{
		return _root;
	}

	static std::shared_ptr<Revision> thread_revision();

#ifdef DEBUG
	[[nodiscard]] std::string dump() const;
#endif
};
