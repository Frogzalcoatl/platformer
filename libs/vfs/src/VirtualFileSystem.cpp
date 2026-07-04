#include "VirtualFileSystem.hpp"
#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <string>

VirtualFileSystem::VirtualFileSystem(
    const std::filesystem::path& basePath,
    const std::filesystem::path& datFileName,
    std::string_view expectedVersion,
    std::string_view assetsFolderName
)
    : basePath(basePath), assetsFolderName(assetsFolderName) {
    std::filesystem::path assetsPath = std::filesystem::path(basePath) / assetsFolderName;
    packFilePath = std::filesystem::path(basePath) / datFileName;
    if (std::filesystem::exists(assetsPath) && std::filesystem::is_directory(assetsPath)) {
        useFolder = true;
    }
    std::ifstream packFile(packFilePath, std::ios::binary);
    if (!packFile.is_open()) {
        if (useFolder) {
            return;
        }
        throw std::runtime_error("Missing dat file\n" + datFileName.string());
    }
    VFS_Header header{};
    packFile.read(reinterpret_cast<char*>(&header), sizeof(VFS_Header));
    if (!packFile) {
        if (useFolder) {
            return;
        }
        throw std::runtime_error("Failed to read header from dat file\n" + datFileName.string());
    }
    if (header.magic != PackMagic) {
        if (useFolder) {
            return;
        }
        throw std::runtime_error("Invalid dat file magic\n" + datFileName.string());
    }
    std::string_view headerVersion(header.version, sizeof(header.version));
    size_t nullPos = headerVersion.find('\0');
    if (nullPos != std::string_view::npos) {
        headerVersion = headerVersion.substr(0, nullPos);
    }
    if (std::string_view(headerVersion) != expectedVersion) {
        if (useFolder) {
            return;
        }
        std::string error = "Invalid dat file version:\nExpected \"";
        error += expectedVersion;
        error += "\"\nFound \"";
        error += headerVersion;
        error += "\"";
        throw std::runtime_error(error);
    }
    entries.resize(header.fileCount);
    if (header.fileCount > 0) {
        packFile.read(reinterpret_cast<char*>(entries.data()), entries.size() * sizeof(VFS_Entry));
        if (!packFile) {
            if (useFolder) {
                entries.clear();
                return;
            }
            throw std::runtime_error("Failed to read VFS entries from " + datFileName.string());
        }
    }
}

std::vector<std::byte> VirtualFileSystem::readFile(std::string_view relativeFilePath) {
    if (useFolder) {
        std::filesystem::path fullFilePath = basePath / assetsFolderName / relativeFilePath;
        if (std::filesystem::exists(fullFilePath) &&
            std::filesystem::is_regular_file(fullFilePath)) {
            // std::ios::ate starts the stream pointer at the end position to immedietely get total
            // file size.
            std::ifstream file(fullFilePath, std::ios::binary | std::ios::ate);
            if (file.is_open()) {
                std::streamsize size = file.tellg();
                file.seekg(0, std::ios::beg);
                std::vector<std::byte> buffer(static_cast<size_t>(size));
                if (file.read(reinterpret_cast<char*>(buffer.data()), size)) {
                    return buffer;
                }
            }
        }
    }
    uint64_t hash = stringHash(relativeFilePath);
    auto iterator = std::lower_bound(
        entries.begin(), entries.end(), hash, [](const VFS_Entry& entry, uint64_t h) {
            return entry.id < h;
        }
    );
    if (iterator != entries.end() && iterator->id == hash) {
        std::ifstream packFile(packFilePath, std::ios::binary);
        if (packFile.is_open()) {
            packFile.seekg(iterator->offset, std::ios::beg);
            std::vector<std::byte> buffer(iterator->size);
            if (packFile.read(reinterpret_cast<char*>(buffer.data()), iterator->size)) {
                return buffer;
            }
        }
    }
    return {};
}