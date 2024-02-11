#include "utils.h"

std::vector<std::string> tokenize(const std::string& str, const char delim, const char ignBracketStart, const char ignBracketEnd)
{
	std::stringstream ss(str);
	std::vector<std::string> tokens;

	std::string token;
	size_t bracketCounter = 0;
	for(char ch : str)
	{
		if(ch == delim && bracketCounter == 0)
		{
			tokens.push_back(token);
			token.clear();
		}
		else
		{
			token.push_back(ch);
		}

		if(ignBracketStart == ch)
			++bracketCounter;
		else if(ignBracketEnd == ch)
			--bracketCounter;
	}
	if(bracketCounter == 0)
		tokens.push_back(token);
	return tokens;
}

std::string stripWhitespace(const std::string& in)
{
	std::string out;
	out.reserve(in.size());
	for(char ch : in)
	{
		if(ch <= 32 || ch == 127)
			continue;
		out.push_back(ch);
	}
	return out;
}

static void recursive_get_matching_files_impl(const std::filesystem::path& path,
                                                                     std::function<bool(const std::filesystem::path&)> pred,
                                                                     std::vector<std::filesystem::path>& paths)
{
	if(!std::filesystem::is_directory(path) && pred(path))
	{
		paths.push_back(path);
	}
	else if(std::filesystem::is_directory(path))
	{
		for(auto const& dirent : std::filesystem::directory_iterator{path})
		{
			try
			{
				recursive_get_matching_files_impl(dirent.path(), pred, paths);
			}
			catch(const std::filesystem::filesystem_error& err)
			{
				continue;
			}
		}
	}
}

std::vector<std::filesystem::path> recursive_get_matching_files(const std::filesystem::path& path,
	std::function<bool(const std::filesystem::path&)> pred)
{
	std::vector<std::filesystem::path> out;
	recursive_get_matching_files_impl(path, pred, out);
	return out;
}
