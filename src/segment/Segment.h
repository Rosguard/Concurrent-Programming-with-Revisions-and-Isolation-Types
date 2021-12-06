#pragma once

#include <memory>
#include <vector>
#include "versioned/Versioned.h"

class Segment {
    private:
	int _version;
	std::shared_ptr<Segment> _parent;
	std::vector<std::shared_ptr<Versioned> > _written;

    public:
	Segment(const std::shared_ptr<Segment> &parent);

	// some getters
	inline std::vector<std::shared_ptr<Versioned> > &written()
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

	// release
	~Segment();
};
