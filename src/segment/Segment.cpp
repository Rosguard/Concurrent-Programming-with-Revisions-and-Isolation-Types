#include <cassert>
#include "Segment.h"


Segment::Segment(const std::shared_ptr<Segment> &parent)
{
	this->_parent = parent;
	_versionCount = 0;
	if (parent != nullptr) {
		parent->_refcount++;
	}
	_version = _versionCount++;
	_refcount = 1;
}

void Segment::collapse(const std::shared_ptr<Revision> &main)
{
	assert(main->current() == shared_from_this());
	while (_parent != nullptr && _parent->_refcount == 1) {
		std::for_each(_parent->_written.begin(),
			      _parent->_written.end(),
			      [this, main](std::shared_ptr<Versioned> &ptr) {
				      ptr->collapse(main, _parent);
			      });
		_parent = _parent->_parent;
	}
}

void Segment::release()
{
	if (--_refcount == 0) {
		std::for_each(_written.begin(), _written.end(),
			      [this](std::shared_ptr<Versioned> &ptr) {
				      ptr->release(shared_from_this());
			      });
	}
	if (_parent != nullptr) {
		_parent->release();
	}
}
