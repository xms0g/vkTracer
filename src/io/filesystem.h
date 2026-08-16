#pragma once
#include <string>
#include <vector>

namespace fs {
std::string path(const std::string& path);
std::vector<char> readFile(const std::string& fileName);
}
