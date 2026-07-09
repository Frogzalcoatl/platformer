#include "FormatLogs.hpp"
#include <format>
#include <iomanip>
#include <iostream>
#include <sstream>

static std::string getTimeStamp() {
    SDL_Time ticks;
    SDL_DateTime dt;
    if (SDL_GetCurrentTime(&ticks) && SDL_TimeToDateTime(ticks, &dt, true)) {
        return std::format("{:02}:{:02}:{:02}", dt.hour, dt.minute, dt.second);
    } else {
        return "00:00:00";
    }
}

static std::string priorityToString(SDL_LogPriority priority) {
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

static std::string categoryToString(int category) {
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

static void sdlOutput(void* userdata, int category, SDL_LogPriority priority, const char* message) {
    (void)userdata;
    if (!message) {
        return;
    }
    std::string timeStamp = getTimeStamp();
    std::cout << "[" << timeStamp << "] [" << categoryToString(category) << "/"
              << priorityToString(priority) << "]: " << message << std::endl;
}

void initSdlLogFormatting() {
    SDL_SetLogOutputFunction(sdlOutput, nullptr);
}