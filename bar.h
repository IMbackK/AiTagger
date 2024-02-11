#pragma once
#include <thread>
#include <indicators/indeterminate_progress_bar.hpp>
#include <indicators/progress_bar.hpp>

std::thread* bar_create_new_indeterminate(const std::string& text);
void bar_stop_indeterminate(std::thread* thread);

indicators::ProgressBar* bar_create_new(std::string text, size_t maxProgress);
