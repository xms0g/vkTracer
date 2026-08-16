#pragma once

struct VertexLayout {
	vk::VertexInputBindingDescription bindingDescription;
	std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
};
