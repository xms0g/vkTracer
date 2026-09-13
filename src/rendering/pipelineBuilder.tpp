#pragma once

template<vk::DynamicState T>
PipelineBuilder& PipelineBuilder::dynamicState() {
	mDynamicStates.push_back(T);
	return *this;
}

template<vk::DynamicState... States>
PipelineBuilder& PipelineBuilder::dynamicStates() {
	(mDynamicStates.push_back(States), ...);
	return *this;
}
