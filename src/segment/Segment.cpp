#include "Segment.h"

std::atomic<int> Segment::_versionCount = 0;

Segment::Segment(const std::shared_ptr<Segment> &parent) : _parent(parent)
{
	if (parent) {
		DEBUG_ONLY("Create Segment with parent: " +
			   _parent->version_to_string());
		parent->_refcount++;
	}
	_version = _versionCount++;
	DEBUG_ONLY("New Segment version: " + version_to_string());
	_refcount = 1;
}

void Segment::collapse(const Revision *main)
{
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
	if (--_refcount == 0) {
		std::for_each(_written.begin(), _written.end(),
			      [this](Versioned *const ptr) {
				      ptr->release(this);
			      });
		if (_parent) {
			_parent->release();
		}
	}
}
