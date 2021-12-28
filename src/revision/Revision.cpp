#include "Revision.h"

thread_local std::shared_ptr<Revision> Revision::_current_revision = nullptr;

Revision::Revision(const std::shared_ptr<Segment> &root,
		   const std::shared_ptr<Segment> &current)
	: _root(root), _current(current)
{
	DEBUG_ONLY("Create Revision: current: " + current->dump() +
		   (root ? " root " + root->dump() : " nullptr"));
}

std::shared_ptr<Revision> Revision::fork(const std::function<void()> &action)
{
	DEBUG_ONLY("Fork: " + dump());

	auto r = std::make_shared<Revision>(
		_current, std::make_shared<Segment>(_current));

	_current->release();
	_current = std::make_shared<Segment>(_current);

	r->_task = std::thread([r, action]() {
		const auto prev_rev = _current_revision;
		_current_revision = r;

		action();

		_current_revision = prev_rev;
	});

	return r;
}

void Revision::join(const std::shared_ptr<Revision> &other_revision)
{
	DEBUG_ONLY("Join: " + dump() + " with " + other_revision->dump());

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
			std::make_shared<Revision>(nullptr, root_segment);
	}

	return _current_revision;
}

#ifdef DEBUG
std::string Revision::dump() const
{
	return "Revision:" + (_root ? " root: " + _root->dump() : "") +
	       " current: " + _current->dump();
}
#endif // DEBUG
