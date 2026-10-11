#include "InputRouter.h"
#include <algorithm>

void InputRouter::push(InputLayer* layer)
{
	layers_.push_back(layer);
}

void InputRouter::remove(InputLayer* layer)
{
	layers_.erase(std::remove(layers_.begin(), layers_.end(), layer), layers_.end());
}

bool InputRouter::OnEvent(const irr::SEvent& event)
{
	state_.update(event);
	for (size_t i = layers_.size(); i > 0; --i)
		if (layers_[i - 1]->isActive() && layers_[i - 1]->onEvent(event))
			return true;
	return false;
}
