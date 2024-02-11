# AITagger

AITagger is an application that sets music/mp3 i3 tags on files based on thair file names.
This a application will take a folder of music files like

* Domination by APOCALYPTICA feat Dead Soul Tirbe.opus
* Apocalyptica - 04 Nothing Else Matters.mp3

and so on and trys to extract what is the album (if possible), the titel and the artist.

The core of this application is a Transformer model based on GPT2 architecture.

## Building

### Requirements

* wget in $PATH
* [llamacpp](https://github.com/ggerganov/llama.cpp)
* [tagparser](https://github.com/Martchus/tagparser)
* a c++20 compiler
* a ROCM or CUDA install and a compatable gpu with at least 4GB vram is recommended


### Procedure

1. clone this repo and change the working directory to where you cloned it
2. `mkdir build; cd build`
3. `cmake .. ; make`
4. `bash get-models.sh`
	* this will download the models and place them in the ./net directory

## Usage

1. Perpare a directory with the music files you would like to tag
2. run aimusictagger with your desired paramters on the directory
	* see --help for a full set of flags
	* you must select network weights to use two options are available
		1. tags-gpt2-medium.gguf is more agressive at guessing tags not apperant in the file names
			* this will likely only work well if the model has seen the title in question before during training
		2. tags-gpt2-medium-v2.gguf is less agressive and gennerally what should probubly be used
3. the tagged files will be placed where you specify with `--out`

