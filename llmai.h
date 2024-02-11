#pragma once

#include <llama.h>
#include <vector>
#include <string>

std::vector<llama_token> generate_text(std::vector<llama_token> prompt, llama_model* model);
std::string llama_untokenize(const std::vector<llama_token>& tokens, const llama_model* model);
std::string llama_token_to_piece(const llama_model* model, llama_token token);
