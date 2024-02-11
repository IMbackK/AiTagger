#include "bar.h"

#include <indicators/setting.hpp>
#include <indicators/cursor_control.hpp>
#include <atomic>

static std::atomic<bool> stopBar = false;

void bar_stop_indeterminate(std::thread* thread)
{
	stopBar = true;
	thread->join();
	delete thread;
	stopBar = false;
}

static void bar_indeterminate_thread(std::string text)
{
	indicators::IndeterminateProgressBar bar{indicators::option::BarWidth{50},
		indicators::option::BarWidth{50},
		indicators::option::Start{"["},
		indicators::option::Fill{"·"},
		indicators::option::Lead{"<==>"},
		indicators::option::End{"]"},
		indicators::option::PostfixText{text},
	};
	//indicators::show_console_cursor(false);
	while(!stopBar)
	{
		bar.tick();
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	indicators::show_console_cursor(true);
}

std::thread* bar_create_new_indeterminate(const std::string& text)
{
	return new std::thread(bar_indeterminate_thread, text);
}

indicators::ProgressBar* bar_create_new(std::string text, size_t maxProgress)
{
	indicators::ProgressBar *bar = new indicators::ProgressBar{
		indicators::option::Start{" ["},
		indicators::option::Fill{"="},
		indicators::option::Lead{">"},
		indicators::option::Remainder{" "},
		indicators::option::End{"]"},
		indicators::option::BarWidth{80},
		indicators::option::MaxProgress{maxProgress},
		indicators::option::PrefixText{text},
		indicators::option::ShowPercentage{true},
		indicators::option::ShowElapsedTime{true},
		indicators::option::ShowRemainingTime{true},
	};
	return bar;
}
