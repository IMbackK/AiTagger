#include <climits>
#include <iostream>
#include <numeric>
#include <vector>
#include <string>
#include <filesystem>
#include <functional>
#include <llama.h>
#include <algorithm>
#include <thread>
#include <cctype>
#include <fstream>

#include "options.h"
#include "utils.h"
#include "bar.h"
#include "llmai.h"
#include "tag.h"

static bool match_music_files(const std::filesystem::path& path)
{
	static const std::vector<std::string> musicExtensions = {".mp3", ".ogg", ".flac", ".opus"};
	auto search = std::find(musicExtensions.begin(), musicExtensions.end(), path.extension());
	return search != musicExtensions.end();
}

static std::vector<llama_token> tokenize_path(const std::filesystem::path& p, llama_model *model, indicators::ProgressBar* bar = nullptr)
{
	constexpr int32_t max_len = 255;
	std::vector<llama_token> out(max_len);

	std::string filename = p.filename().string();

	int32_t ret = llama_tokenize(model, filename.c_str(), filename.length(), out.data(), out.size(), false, false);

	if(ret < 0)
		out.clear();

	out.resize(ret);

	if(bar)
		bar->tick();

	return out;
}

std::vector<std::string> get_responses_for_paths(llama_model* model, const std::vector<std::filesystem::path>& prompts)
{
	auto* bar = bar_create_new("Tokenizeing prompts", prompts.size());
	std::vector<std::vector<llama_token>> tokenizedPrompts(prompts.size());
	std::transform(prompts.begin(), prompts.end(), tokenizedPrompts.begin(), [model, bar](const std::filesystem::path& p){return tokenize_path(p, model, bar);});
	delete bar;

	std::vector<std::string> responses(prompts.size());

	bar = bar_create_new("Generating responses", prompts.size());
	for(size_t i = 0; i < prompts.size(); ++i)
	{
		std::vector<llama_token> tokens = generate_text(tokenizedPrompts[i], model);
		responses[i] = llama_untokenize(tokens, model);
		bar->tick();
	}
	delete bar;

	return responses;
}

void drop_log(enum ggml_log_level level, const char* text, void* user_data)
{
	(void)level;
	(void)text;
	(void)user_data;
}


std::vector<AiTags> parse_tags_from_responses(const std::vector<std::string>& responses, const std::vector<std::filesystem::path>& paths)
{
	assert(responses.size() == paths.size());

	std::vector<AiTags> out(responses.size());
	auto bar = bar_create_new("Processing responses", responses.size());
	#pragma omp parallel for
	for(size_t i = 0; i < responses.size(); ++i)
	{
		out[i].parseFromResponse(responses[i], paths[i]);
		bar->tick();
	}
	delete bar;

	return out;
}

int main(int argc, char** argv)
{
	Config config = get_arguments(argc, argv);

	if(!config.debug)
		llama_log_set(drop_log, nullptr);
	llama_backend_init(false);
	llama_model_params modelParams = llama_model_default_params();
	modelParams.n_gpu_layers = config.gpu ? 1000 : 0;
	llama_model* model = llama_load_model_from_file(config.model.c_str(), modelParams);

	if(!model)
	{
		std::cerr<<"Unable to load model from "<<config.model<<'\n';
		return 2;
	}

	std::thread* barThread = bar_create_new_indeterminate("Searching for music files");
	std::vector<std::filesystem::path> muiscPaths = recursive_get_matching_files(config.in, match_music_files);
	bar_stop_indeterminate(barThread);

	std::vector<std::string> responses = get_responses_for_paths(model, muiscPaths);
	llama_free_model(model);
	llama_backend_free();

	if(config.debug)
	{
		std::ofstream file;
		file.open("./debug.log");
		if(file.is_open())
		{
			for(size_t i = 0; i < responses.size(); ++i)
				file<<muiscPaths[i]<<":\n"<<responses[i]<<"\n\n";
			file.close();
		}
		else
		{
			std::cerr<<"could not open ./debug.log for writeing";
		}
	}

	std::vector<AiTags> aiTags = parse_tags_from_responses(responses, muiscPaths);

	if(!std::filesystem::is_directory(config.out) && !std::filesystem::create_directory(config.out))
	{
		std::cerr<<config.out<<" is not a directory and a directory could not be created at this location\n";
		return 1;
	}


	if(!config.rejectDir.empty() && !std::filesystem::is_directory(config.rejectDir) && !std::filesystem::create_directory(config.rejectDir))
	{
		std::cerr<<config.rejectDir<<" is not a directory and a directory could not be created at this location\n";
		return 1;
	}

	auto bar = bar_create_new("Writeing tags to disk", aiTags.size());

	#pragma omp parallel for
	for(AiTags& tag : aiTags)
	{
		bar->tick();
		if(!tag.isFilled())
		{
			std::error_code ec;
			std::filesystem::copy(tag.path, config.rejectDir, ec);
			if(ec)
				std::cerr<<ec.message()<<'\n';
			continue;
		}


		std::filesystem::path artistDir;
		if(config.flatOutput)
		{
			artistDir = config.out;
		}
		else
		{
			artistDir = config.out/tag.artist;
			if(!std::filesystem::is_directory(artistDir) && !std::filesystem::create_directory(artistDir))
			{
				std::cerr<<artistDir<<" is not a directory and a directory could not be created at this location\n";
				return 1;
			}
		}

		std::error_code ec;
		std::filesystem::copy(tag.path, artistDir, ec);
		if(ec)
		{
			std::cerr<<ec.message()<<'\n';
			continue;
		}
		tag.path = artistDir/tag.path.filename();

		tag.writeToFile();
	}

	delete bar;

	return 0;
}

