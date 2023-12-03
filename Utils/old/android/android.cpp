#include "../base.h"


JavaVM *g_jvm = nullptr;

void jvm_init(JNIEnv* env) {
    env->GetJavaVM(&g_jvm);
}
