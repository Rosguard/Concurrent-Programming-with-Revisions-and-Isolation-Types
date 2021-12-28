#pragma once

#include <memory>

#ifdef DEBUG
#include <iostream>
#define DEBUG_ONLY(str) std::cerr << str << std::endl;
#else
#define DEBUG_ONLY(str)
#endif

class Revision;
class Segment;

class Versioned {
    public:
	virtual void release(const Segment *) = 0;

	virtual void collapse(const Revision *,
			      const std::shared_ptr<Segment> &) = 0;

	virtual void merge(const Revision *, const std::shared_ptr<Revision> &,
			   const std::shared_ptr<Segment> &) = 0;

	virtual ~Versioned() = default;
};
