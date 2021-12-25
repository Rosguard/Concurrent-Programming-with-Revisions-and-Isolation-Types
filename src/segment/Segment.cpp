#include "Segment.h"

std::atomic<int> Segment::_versionCount = 0;

Segment::Segment(const std::shared_ptr<Segment> &parent) : _parent(parent)
{
	if (parent) {
		parent->_refcount++;
	}
	_version = _versionCount++;
	_refcount = 1;

	DEBUG_ONLY("Create Segment: " + dump() +
		   (parent ? " with parent " + parent->dump() : ""));
}

void Segment::collapse(const Revision *main)
{
	DEBUG_ONLY("Collapse Segment: " + main->dump());

	assert(main->current().get() == this);

	while (_parent != main->root() && _parent->_refcount == 1) {
		std::for_each(_parent->_written.begin(),
			      _parent->_written.end(),
			      [this, main](Versioned *const ptr) {
				      ptr->collapse(main, _parent);
			      });
		_parent = _parent->_parent;
	}
}

void Segment::release()
{
	DEBUG_ONLY("Release Segment: " + dump());

	if (--_refcount == 0) {
		DEBUG_ONLY("Full release Segment: " + dump());

		std::for_each(_written.begin(), _written.end(),
			      [this](Versioned *const ptr) {
				      ptr->release(this);
			      });
		if (_parent) {
			_parent->release();
		}
	}
}
