#pragma once

#include <memory>

class Revision;
class Segment;

class Versioned : public std::enable_shared_from_this<Versioned> {
    public:
	virtual void release(const std::shared_ptr<Segment> &) = 0;

	virtual void collapse(const std::shared_ptr<Revision> &,
			      const std::shared_ptr<Segment> &) = 0;

	virtual void merge(const std::shared_ptr<Revision> &,
			   const std::shared_ptr<Revision> &,
			   const std::shared_ptr<Segment> &) = 0;
};
