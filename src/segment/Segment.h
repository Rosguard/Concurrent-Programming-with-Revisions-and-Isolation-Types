#pragma once

#include <memory>
#include <thread>
#include <functional>
#include <utility>
#include <atomic>
#include "versioned/Versioned.h"
#include "revision/Revision.h"

class Segment : public std::enable_shared_from_this<Segment> {
    private:
	std::atomic<int> _version{};
	std::atomic<int> _refcount{};
	std::atomic<int> _versionCount{};

	std::shared_ptr<Segment> _parent;

	std::vector<std::shared_ptr<Versioned> > _written;

    public:
	Segment(const std::shared_ptr<Segment> &parent);

	// some getters
	inline std::vector<std::shared_ptr<Versioned> > written()
	{
		return _written;
	}

	[[nodiscard]] inline std::shared_ptr<Segment> parent() const
	{
		return _parent;
	}

	[[nodiscard]] inline int version() const
	{
		return _version;
	}

	void collapse(const std::shared_ptr<Revision> &main);
	void release();

	// release
	~Segment();
};
