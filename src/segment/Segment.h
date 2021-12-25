#pragma once

#include <memory>
#include <thread>
#include <functional>
#include <utility>
#include <atomic>
#include "versioned/Versioned.h"
#include "revision/Revision.h"
#include <cassert>

class Segment {
    private:
	std::atomic<int> _version{};
	std::atomic<int> _refcount{};
	static std::atomic<int> _versionCount;

	std::shared_ptr<Segment> _parent;
	std::vector<Versioned *> _written;

    public:
	explicit Segment(const std::shared_ptr<Segment> &parent);

	// some getters
	inline std::vector<Versioned *> &written()
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

	void collapse(const Revision *main);
	void release();

#ifdef DEBUG
	[[nodiscard]] std::string version_to_string() const
	{
		return std::to_string(version());
	}
#endif
};
