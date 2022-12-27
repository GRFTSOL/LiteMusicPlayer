#pragma once

#include <jni.h>
#include <android/log.h>


extern JavaVM *g_jvm;

void jvm_init(JNIEnv* env);
