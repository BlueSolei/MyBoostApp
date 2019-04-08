#include <jni.h>
#include <string>

#include <memory>

#include "SharedObject.hpp"
#include "defer.hpp"
#include "AndroidLog.h"

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_myboostapp_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++ fd is: shaul";
    return env->NewStringUTF(hello.c_str());
}

struct Payload
{
  std::atomic<int> debuggerExist;
};
using SharedPayload = SharedObject<Payload>;
using SharedPayloadUPtr = std::unique_ptr<SharedPayload>;

SharedPayloadUPtr g_payload;

SharedPayload* CreateSharedObject(int fd)
{
  if(g_payload) return g_payload.get();

  try {
    g_payload = std::make_unique<SharedPayload>(fd);
  }
  catch(std::exception& e)
  {
    LOGE("Failed to create shared object. error: '%s'", e.what());
  }
  catch(...)
  {
    LOGE("Failed to create shared object. error: '<unknown>'");
  }

  return g_payload.get();
}

SharedPayload* CreateSharedObject(const std::string& name, SharedPayload::Profile profile)
{
  if(g_payload) return g_payload.get();

  try {
    g_payload = std::make_unique<SharedPayload>(name, profile);
  }
  catch(std::exception& e)
  {
    LOGE("Failed to create shared object. error: '%s'", e.what());
  }
  catch(...)
  {
    LOGE("Failed to create shared object. error: '<unknown>'");
  }

  return g_payload.get();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_example_myboostapp_SharedObject_nativeGetValue(JNIEnv *env,
                                                        jobject /* this */,
                                                        jstring sharedObjectName)
{
  const char *name = env->GetStringUTFChars(sharedObjectName, NULL);
  defer{ env->ReleaseStringUTFChars(sharedObjectName, name); };
  LOGD("GET: C++ get value for object '%s'", name);
  auto payload = CreateSharedObject(name, SharedPayload::Profile::user);
  if(!payload) return -1;

  LOGD("GET: C++ acquire read object '%s'", name);
  auto handle = payload->Acquire();
  LOGD("GET: C++ object '%s' value is %d", name, (int)handle->debuggerExist);
  return handle->debuggerExist;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_myboostapp_SharedObject_nativeSetValue(JNIEnv *env,
                                                        jobject /* this */,
                                                        jstring sharedObjectName,
                                                        jint value)
{
  const char *name = env->GetStringUTFChars(sharedObjectName, NULL);
  defer{ env->ReleaseStringUTFChars(sharedObjectName, name); };
  LOGD("SET: C++ set value for object '%s', to %d", name, value);
  auto payload = CreateSharedObject(name, SharedPayload::Profile::creator);
  if(!payload) return;

  LOGD("SET: C++ acquire write object '%s'", name);
  auto handle = payload->Acquire();
  handle->debuggerExist = value;
  LOGD("SET: C++ object '%s' value is %d", name, (int)handle->debuggerExist);
}

extern "C" JNIIMPORT int JNICALL
Java_com_example_myboostapp_SharedObject_getSharedObjectFD(JNIEnv *env,
                                               jobject /* this */,
                                               jstring sharedObjectName)
{
  const char *name = env->GetStringUTFChars(sharedObjectName, NULL);
  defer{ env->ReleaseStringUTFChars(sharedObjectName, name); };
  LOGD("CreateByName: C++ object '%s'", name);
  auto payload = CreateSharedObject(name, SharedPayload::Profile::creator);
  auto fd = (payload ? payload->FD() : -1);
  LOGD("CreateByName: C++ object '%s', fd is ", name, fd);
  return fd;
}

extern "C" JNIIMPORT void JNICALL
Java_com_example_myboostapp_SharedObject_createSharedObject(JNIEnv *env,
                                                            jobject /* this */,
                                                            jint fd)
{
  LOGD("CreateFromFD: C++ object for fd: %d", fd);
  CreateSharedObject(fd);
}