#include "filesystem.h"
#include <filesystem>
#include <fstream>

std::string fs::path(const std::string& path) {
	auto cwd = std::filesystem::current_path().parent_path();
	return cwd.append(path).string();
}

std::vector<char> fs::readFile(const std::string& fileName) {
    std::ifstream file{fileName, std::ios::binary | std::ios::ate};

    if (!file.is_open()) {
        throw std::runtime_error("Failed to read file: " + fileName);
    }

    const size_t fileSize = file.tellg();
    std::vector<char> fileBuffer(fileSize);

    file.seekg(0);
    file.read(fileBuffer.data(), static_cast<std::streamsize>(fileSize));
    file.close();

    return fileBuffer;
}
