#include <jni.h>
#include <string>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/ashmem.h>

#include "SharedObject.hpp"

struct Payload
{
    int counter = 0;
};
using SharedPayload = SharedObject<Payload>;

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_myboostapp_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++ fd is: shaul";
    try {
        SharedPayload payload("payload", SharedPayload::Profile::creator);
    }
    catch(std::exception& e)
    {
        hello += std::string("Exception: ") + e.what();
    }
    catch(...)
    {
        hello += std::string("Exception: <...>");
    }
    return env->NewStringUTF(hello.c_str());
}
