#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <functional>

std::vector<std::string> tokenize(const std::string& str, const char delim = ' ', const char ignBracketStart = '\0', const char ignBracketEnd = '\0');

std::string stripWhitespace(const std::string& in);

std::vector<std::filesystem::path> recursive_get_matching_files(const std::filesystem::path& path,
	std::function<bool(const std::filesystem::path&)> pred = [](const std::filesystem::path&) -> bool{return true;});

