#pragma once

#include "versioned/Versioned.h"
#include "segment/Segment.h"
#include "revision/Revision.h"
#include <unordered_map>
#include <optional>

template <class T> class VDataStructure : public Versioned {
    protected:
	std::unordered_map<int, std::optional<T> > _versions;

	void set(const std::shared_ptr<Revision> &r, const T &value);
	T &get(const std::shared_ptr<Revision> &r);

	void set(const T &value);
	T &get();

    public:
	void release(const std::shared_ptr<Segment> &release) override;

	void collapse(const std::shared_ptr<Revision> &main,
		      const std::shared_ptr<Segment> &parent) override;

	void merge(const std::shared_ptr<Revision> &main,
		   const std::shared_ptr<Revision> &joinRev,
		   const std::shared_ptr<Segment> &join) override;
};
