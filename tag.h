#pragma once
#include <filesystem>
#include <string>

class AiTags
{
private:
	static bool formatCandiateTag(std::string& tag);
	static bool checkCandiateTag(std::string tag, const std::filesystem::path& path);

public:
	std::filesystem::path path;
	std::string artist;
	std::string title;
	std::string album;

	bool isFilled() const;
	void parseFromResponse(const std::string& response, const std::filesystem::path &path, bool noAlbum = false);
	bool writeToFile() const;
};
