#include <jni.h>
#include <string>

#include <memory>
#include <regex>

#define ANDROID_LOG_TAG "SharedObject"
#include "SharedObject.hpp"
#include "defer.hpp"
#include "Mine/AndroidLog.h"
#include "Mine/expected.hpp"


tl::expected<std::string, std::string> ExhostPipe(std::string command)
{
  std::string output;
  FILE *fpipe = nullptr;
  char line[0x1 << 10];
  if ( !(fpipe = (FILE*)popen(command.c_str(),"r")) )
  {
    sscanf(line, "%s", strerror(errno));
    return tl::make_unexpected(line);
  }
  else
  {
    while (fgets( line, sizeof line, fpipe))
    {
      output += line;
    }
    pclose(fpipe);
  }
  return output;
}

std::string ListPS()
{
  auto output = ExhostPipe("/system/bin/ps");
  return output ? output.value() : std::string("Error: ") + output.error();
}

std::string ListProc(int pid, int stream)
{
  auto output = ExhostPipe("ls /proc/" + std::to_string(pid) + "/");
  return output.value_or(std::string() + "Error: " + output.error());
}

tl::expected<int, std::string> GetPID(const std::string& procName)
{
  auto pid = ExhostPipe("/system/bin/ps")
          .and_then([&](const std::string& proccessList) -> tl::expected<std::string,std::string>
               {
                 std::regex regexPID(R"(\w+ +(\d+) +\d+ +\d+ +\d+ +\w+ +\w+ +\w +)" + procName);
                 std::smatch matchPID;
                 if(std::regex_search(proccessList, matchPID, regexPID) && (matchPID.size() == 2))
                 {
                   std::string strPID = matchPID[1].str();
                   return strPID;
                 } else{
                   return tl::make_unexpected(std::string() + "Failed to find process '" + procName + "' in process list");
                 }
               })
          .and_then([](const std::string& strPID) -> tl::expected<int, std::string>
               {
                 try {
                   std::size_t count = 0;
                   int pid = std::stoi(strPID, &count);
                   if(count != strPID.size()) {
                     throw std::invalid_argument("");
                   }
                   return pid;
                 }
                 catch(std::exception& /*e*/)
                 {
                   return tl::make_unexpected(std::string() + "PID '" + strPID + "' is ill formed");
                 }
               });
  return pid;
}

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

SharedPayload* CreateSharedObject(const std::string& name, SharedPayload::Profile profile)
{
  if(g_payload) return g_payload.get();

  try {
    g_payload = std::make_unique<SharedPayload>(profile);
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

extern "C" JNIEXPORT void JNICALL
Java_com_example_myboostapp_SharedObject_nativeStopServer(JNIEnv *env,
                                                          jobject /* this */,
                                                          jstring sharedObjectName)
{
  LOGD("SET: C++ server is stopping ...");
  g_payload.reset();
  LOGD("SET: C++ server has stopped");
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_myboostapp_Communicate_nativePS(JNIEnv* env, jobject /*this*/)
{
  auto pid = GetPID("com.example.myboostapp:other");
  auto streams = pid.map([](int pid) { return std::to_string(pid); });
//  auto streams = pid.and_then([](int pid) { return ExhostPipe("ls /proc/" + std::to_string(pid) + "/fd"); });
  auto strPID = std::string() + "streams of service are: [" + streams.value_or("-1") + "]";
  return env->NewStringUTF(strPID.c_str());
}
