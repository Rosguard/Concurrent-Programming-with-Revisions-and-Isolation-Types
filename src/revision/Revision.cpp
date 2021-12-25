#include "Revision.h"

thread_local std::shared_ptr<Revision> Revision::_current_revision = nullptr;

std::shared_ptr<Revision> Revision::fork(const std::function<void()> &action)
{
	DEBUG_ONLY("Fork with Segment: " + _current->version_to_string());

	auto r = std::make_shared<Revision>(
		_current, std::make_shared<Segment>(_current));

	_current = std::make_shared<Segment>(_current);

	r->_task = std::thread([r, action]() {
		DEBUG_ONLY("Start new thread!");

		const auto prev_rev = _current_revision;
		_current_revision = r;

		action();

		_current_revision = prev_rev;
	});

	return r;
}

void Revision::join(const std::shared_ptr<Revision> &other_revision)
{
	other_revision->_task.join();

	auto s = other_revision->_current;
	while (s != other_revision->_root) {
		DEBUG_ONLY("Possible merges: " +
			   std::to_string(s->written().size()));
		std::for_each(s->written().begin(), s->written().end(),
			      [this, other_revision, s](Versioned *const ptr) {
				      ptr->merge(this, other_revision, s);
			      });
		s = s->parent();
	}

	other_revision->_current->release();
	_current->collapse(this);
}

std::shared_ptr<Revision> Revision::thread_revision()
{
	if (!_current_revision) {
		const auto root_segment = std::make_shared<Segment>(nullptr);
		_current_revision =
			std::make_shared<Revision>(root_segment, root_segment);
	}

	return _current_revision;
}
