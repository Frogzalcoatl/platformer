#include "Android.hpp"
#include <SDL3/SDL.h>

#ifdef SDL_PLATFORM_ANDROID
#include <jni.h>
#endif

void Android::quitAndRemoveTask() {
#ifdef SDL_PLATFORM_ANDROID
    JNIEnv* env = (JNIEnv*)SDL_GetAndroidJNIEnv();
    if (!env) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "JNIEnv is not available to run quitTask");
        return;
    }
    jobject activity = (jobject)SDL_GetAndroidActivity();
    if (!activity) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Android Activity object is not available");
        return;
    }
    jclass activityClass = env->GetObjectClass(activity);
    if (!activityClass) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to get android activity class");
        env->DeleteLocalRef(activity);
        return;
    }
    jmethodID finishMethodId = env->GetMethodID(activityClass, "finishAndRemoveTask", "()V");
    if (!finishMethodId) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to find finishAndRemoveTask method.");
        env->DeleteLocalRef(activityClass);
        env->DeleteLocalRef(activity);
        return;
    }
    SDL_Log("Requesting android to finish and remove task from recents");
    env->CallVoidMethod(activity, finishMethodId);
    env->DeleteLocalRef(activityClass);
    env->DeleteLocalRef(activity);
#endif
}