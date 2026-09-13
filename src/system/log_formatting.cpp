#include "system/log_formatting.h"
#include <format>

static SDL_LogOutputFunction default_sdl_log_func = nullptr;

#ifndef SDL_PLATFORM_ANDROID
static std::string get_timestamp() {
    SDL_Time ticks;
    SDL_DateTime dt;
    if (SDL_GetCurrentTime(&ticks) && SDL_TimeToDateTime(ticks, &dt, true)) {
        return std::format("{:02}:{:02}:{:02}", dt.hour, dt.minute, dt.second);
    } else {
        return "00:00:00";
    }
}
#endif

static std::string priority_to_string(SDL_LogPriority priority) {
    switch (priority) {
    case SDL_LOG_PRIORITY_INVALID:
        return "INVALID";
    case SDL_LOG_PRIORITY_TRACE:
        return "TRACE";
    case SDL_LOG_PRIORITY_VERBOSE:
        return "VERBOSE";
    case SDL_LOG_PRIORITY_DEBUG:
        return "DEBUG";
    case SDL_LOG_PRIORITY_INFO:
        return "INFO";
    case SDL_LOG_PRIORITY_WARN:
        return "WARN";
    case SDL_LOG_PRIORITY_ERROR:
        return "ERROR";
    case SDL_LOG_PRIORITY_CRITICAL:
        return "CRITICAL";
    default:
        return "UNKNOWN";
    }
}

static std::string category_to_string(int category) {
    switch (category) {
    case SDL_LOG_CATEGORY_APPLICATION:
        return "APPLICATION";
    case SDL_LOG_CATEGORY_ERROR:
        return "ERROR";
    case SDL_LOG_CATEGORY_ASSERT:
        return "ASSERT";
    case SDL_LOG_CATEGORY_SYSTEM:
        return "SYSTEM";
    case SDL_LOG_CATEGORY_AUDIO:
        return "AUDIO";
    case SDL_LOG_CATEGORY_VIDEO:
        return "VIDEO";
    case SDL_LOG_CATEGORY_RENDER:
        return "RENDER";
    case SDL_LOG_CATEGORY_INPUT:
        return "INPUT";
    case SDL_LOG_CATEGORY_TEST:
        return "TEST";
    case SDL_LOG_CATEGORY_GPU:
        return "GPU";
    default:
        return "UNKNOWN";
    }
}

static void
sdl_output(void* userdata, int category, SDL_LogPriority priority, const char* message) {
    (void)userdata;
    if (!message) {
        return;
    }
    std::string category_str = category_to_string(category);
    std::string priority_str = priority_to_string(priority);
#ifdef SDL_PLATFORM_ANDROID
    // Dont add timestamp on android since its already included by logcat
    std::string formatted_message = "[" + category_str + "/" + priority_str + "]: " + message;
#else
    std::string time_stamp = get_timestamp();
    std::string formatted_message =
        "[" + time_stamp + "]" + " [" + category_str + "/" + priority_str + "]: " + message;
#endif
    if (default_sdl_log_func) {
        default_sdl_log_func(userdata, category, priority, formatted_message.c_str());
    }
}

void init_sdl_log_formatting() {
    for (int i = 0; i < SDL_LOG_PRIORITY_COUNT; i++) {
        SDL_SetLogPriorityPrefix(static_cast<SDL_LogPriority>(i), nullptr);
    }
    default_sdl_log_func = SDL_GetDefaultLogOutputFunction();
    SDL_SetLogOutputFunction(sdl_output, nullptr);
}