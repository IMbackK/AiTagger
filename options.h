#pragma once

#include <iostream>
#include <filesystem>
#include "argparse.h"

struct Config
{
	bool flatOutput;
	bool noAlbum;
	bool debug;
	bool gpu;
	std::filesystem::path model;
	std::filesystem::path out;
	std::filesystem::path in;
	std::filesystem::path rejectDir;
};

static Config get_arguments(int argc, char** argv)
{
	Config config;
	argparse::ArgumentParser parser(PROG_NAME);

	parser.add_argument("-n", "--no-album").help("don't try to figure out the album").flag();
	parser.add_argument("-f", "--flat").help("output the tagged files into a flat directory").flag();
	parser.add_argument("-d", "--debug").help("output debug infomation").flag();
	parser.add_argument("-g", "--gpu").help("use gpu").flag();
	parser.add_argument("PATH").help("a directory where the music files are stored");
	parser.add_argument("-r", "--reject").help("place to copy the files that where not tagged");
	parser.add_argument("-o", "--out").help("the directory where the tagged files are to be placed").default_value("out");
	parser.add_argument("-m", "--model").help("the ai model to use").required();

	try
	{
		parser.parse_args(argc, argv);
	}
	catch (const std::exception& err)
	{
		std::cerr<<err.what()<<std::endl;
		std::cerr<<parser;
		exit(1);
	}

	config.noAlbum = parser.get<bool>("--no-album");
	config.flatOutput = parser.get<bool>("--flat");
	config.debug = parser.get<bool>("--debug");
	config.gpu = parser.get<bool>("--gpu");
	config.in = parser.get<std::string>("PATH");
	config.out = parser.get<std::string>("--out");
	config.rejectDir = parser.get<std::string>("--reject");
	config.model = parser.get<std::string>("--model");

	return config;
}
