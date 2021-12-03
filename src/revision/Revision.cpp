#include "Revision.h"

std::shared_ptr<Revision> Revision::fork(const std::function<void()> &action)
{
	auto r = std::make_shared<Revision>(
		_current, std::make_shared<Segment>(_current));

	_current = std::make_shared<Segment>(_current);

	r->_task = std::thread([r, &action]() {
		const auto prev_rev = Revision::_current_revision;
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
		std::for_each(s->written().begin(), s->written().end(),
			      [this, other_revision,
			       s](std::shared_ptr<Versioned> &ptr) {
				      ptr->merge(shared_from_this(),
						 other_revision, s);
			      });
		s = s->parent();
	}

	other_revision->_current = nullptr; // release
	_current->collapse(shared_from_this());
}
