#include "llmai.h"

#include <iostream>
#include <cassert>

std::string llama_token_to_piece(const llama_model* model, llama_token token)
{
	std::vector<char> result(8, 0);
	const int n_tokens = llama_token_to_piece(model, token, result.data(), result.size());
	if (n_tokens < 0)
	{
		result.resize(-n_tokens);
		int check = llama_token_to_piece(model, token, result.data(), result.size());
		assert(check == -n_tokens);
	} else
	{
		result.resize(n_tokens);
	}

	return std::string(result.data(), result.size());
}

std::string llama_untokenize(const std::vector<llama_token>& tokens, const llama_model* model)
{
	std::string out;
	out.reserve(tokens.size()*2);
	for(const llama_token token : tokens)
		out.append(llama_token_to_piece(model, token));
	return out;
}

static void llama_batch_add(llama_batch& batch, llama_token id, llama_pos pos, const std::vector<llama_seq_id> &seq_ids, bool logits)
{
	batch.token   [batch.n_tokens] = id;
	batch.pos     [batch.n_tokens] = pos;
	batch.n_seq_id[batch.n_tokens] = seq_ids.size();
	for (size_t i = 0; i < seq_ids.size(); ++i)
		batch.seq_id[batch.n_tokens][i] = seq_ids[i];
	batch.logits  [batch.n_tokens] = logits;

	batch.n_tokens++;
}

std::vector<llama_token> generate_text(std::vector<llama_token> prompt, llama_model* model, int batch_size)
{
	std::vector<llama_token> out;
	llama_context_params ctx_params = llama_context_default_params();
	ctx_params.n_ctx = 2048;
	ctx_params.n_threads = 16;
	ctx_params.n_threads_batch = 16;

	llama_context *ctx = llama_new_context_with_model(model, ctx_params);
	if(!ctx)
	{
		std::cerr<<"Unable to create context\n";
		return out;
    }

	llama_batch batch = llama_batch_init(1024*batch_size, 0, 1);

	batch.n_tokens = 0;
	for (size_t i = 0; i < prompt.size(); i++)
		llama_batch_add(batch, prompt[i], i, { 0 }, false);
	batch.logits[batch.n_tokens - 1] = true;

	int32_t ret = llama_decode(ctx, batch);

	if(ret != 0)
	{
		std::cout<<"WARNING: unable to decode context\n";
		return out;
	}

	int n_cur    = batch.n_tokens;
	int n_decode = 0;
	while (n_cur <= 1024)
	{
		{
			auto   n_vocab = llama_n_vocab(model);
			auto * logits  = llama_get_logits_ith(ctx, batch.n_tokens - 1);

			std::vector<llama_token_data> candidates;
			candidates.reserve(n_vocab);

			for (llama_token token_id = 0; token_id < n_vocab; token_id++) {
				candidates.emplace_back(llama_token_data{ token_id, logits[token_id], 0.0f });
			}

			llama_token_data_array candidates_p = {candidates.data(), candidates.size(), false};

			// sample the most likely token
			const llama_token new_token_id = llama_sample_token_greedy(ctx, &candidates_p);

			// is it an end of stream?
			if (new_token_id == llama_token_eos(model))
				break;

			out.push_back(new_token_id);

			// prepare the next batch
			batch.n_tokens = 0;

			// push this new token for next evaluation
			llama_batch_add(batch, new_token_id, n_cur, { 0 }, true);

			n_decode += 1;
		}

		n_cur += 1;

		ret = llama_decode(ctx, batch);
		if(ret != 0)
		{
			std::cout<<"WARNING: unable to decode context\n";
			break;
		}
	}

	llama_batch_free(batch);
    llama_free(ctx);

	return out;
}
