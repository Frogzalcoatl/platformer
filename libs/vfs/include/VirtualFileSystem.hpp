#pragma once
#include "Packer.hpp"
#include <filesystem>
#include <vector>

class VirtualFileSystem {
  private:
    std::filesystem::path packFilePath;
    std::vector<VFS_Entry> entries;
    const std::filesystem::path basePath;
    bool useFolder = false;

  public:
    VirtualFileSystem(
        const std::filesystem::path& basePath, const std::filesystem::path& datFileName
    );
    // Should be path.generic_string() with forward slashes
    std::vector<std::byte> readFile(std::string_view relativeFilePath);
};