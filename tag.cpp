#include "tag.h"

#include <algorithm>
#include <filesystem>
#include <tagparser/abstractattachment.h>
#include <tagparser/diagnostics.h>
#include <tagparser/mediafileinfo.h>
#include <tagparser/progressfeedback.h>
#include <tagparser/tag.h>
#include <tagparser/tagvalue.h>


#include "utils.h"


bool AiTags::isFilled() const
{
	return !artist.empty() && !title.empty();
}

void AiTags::parseFromResponse(const std::string& response, const std::filesystem::path &path, bool noAlbum)
{
	this->path = path;
	std::vector<std::string> lines = tokenize(response, '\n');
	for(std::string& line : lines)
	{
		std::string* target;
		if(line.starts_with("artist:"))
			target = &artist;
		else if(line.starts_with("title:"))
			target = &title;
		else if(line.starts_with("album:") && !noAlbum)
			target = &album;
		else
			continue;

		formatCandiateTag(line);
		if(checkCandiateTag(line, path))
			*target = line;
	}
}

bool AiTags::writeToFile() const
{
	TagParser::MediaFileInfo fileInfo;
	TagParser::Diagnostics diag;
	TagParser::AbortableProgressFeedback prog;

	try
	{
		fileInfo.setPath(path);
		fileInfo.open();

		fileInfo.parseEverything(diag, prog);
		fileInfo.createAppropriateTags();

		for(TagParser::Tag* tag : fileInfo.tags())
		{
			if(!title.empty())
				tag->setValue(TagParser::KnownField::Title, TagParser::TagValue(title, TagParser::TagTextEncoding::Utf8, tag->proposedTextEncoding()));
			if(!artist.empty())
				tag->setValue(TagParser::KnownField::Artist, TagParser::TagValue(artist, TagParser::TagTextEncoding::Utf8, tag->proposedTextEncoding()));
			if(!album.empty())
				tag->setValue(TagParser::KnownField::Album, TagParser::TagValue(album, TagParser::TagTextEncoding::Utf8, tag->proposedTextEncoding()));
		}

		fileInfo.applyChanges(diag, prog);
		std::filesystem::path backupPath = path;
		backupPath+=std::filesystem::path(".bak");
		std::filesystem::remove(backupPath);
	}
	catch(std::ios_base::failure& err)
	{
		return false;
	}

	return true;
}

bool AiTags::formatCandiateTag(std::string& tag)
{
	std::vector<std::string> tokens = tokenize(tag, ':');
	if(tokens.size() < 2)
		return false;
	tag = tokens[1];
	tag.erase(0, 1);

	return true;
}

bool AiTags::checkCandiateTag(std::string tag, const std::filesystem::path& path)
{
	std::string pathStr = path.string();
	std::transform(tag.begin(), tag.end(), tag.begin(), [](unsigned char c){ return std::tolower(c); });
	std::transform(pathStr.begin(), pathStr.end(), pathStr.begin(), [](unsigned char c){ return std::tolower(c); });

	auto search = pathStr.find(tag);
	return search != std::string::npos;
}
