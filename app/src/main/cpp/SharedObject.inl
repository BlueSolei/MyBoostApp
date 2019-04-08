#pragma once

#include <errno.h>
#include <fcntl.h>
#include <linux/ashmem.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <system_error>

#include "AndroidLog.h"

#define ASHMEM_NAME_LEN         256
#define __ASHMEMIOC             0x77
#define ASHMEM_SET_NAME         _IOW(__ASHMEMIOC, 1, char[ASHMEM_NAME_LEN])
#define ASHMEM_SET_SIZE         _IOW(__ASHMEMIOC, 3, size_t)

namespace {
namespace detail {

template <class T>
T ValidateCall(const T returnValue, const T errorReturnValue = T(-1)) {
  if (returnValue == errorReturnValue) {
    throw std::system_error(errno, std::system_category(), strerror(errno));
  }
  return returnValue;
}
} // namespace detail
} // namespace

template <class T>
SharedObject<T>::SharedObject(std::string objectName, Profile profile,
                              std::string sharedMemoryFolder)
    : m_sharedMemoryName(objectName + "_memory"),
      m_sharedMemoryFolder(sharedMemoryFolder.empty() ? /*ASHMEM_NAME_DEF*/"/dev/ashmem"
                                                      : sharedMemoryFolder),
      m_profile(profile) {
  using namespace detail;
  enum { memorySize = sizeof(SyncedObject) };

  m_sharedMemoryFD = ValidateCall(open(SharedMemoryFolder(), O_RDWR), -1);
  LOGD("Ctor: open returned fd: %d", m_sharedMemoryFD);
  ValidateCall(ioctl(m_sharedMemoryFD, ASHMEM_SET_NAME, SharedMemoryName()));
  ValidateCall(ioctl(m_sharedMemoryFD, ASHMEM_SET_SIZE, memorySize));
  m_sharedMemory =
      (char *)ValidateCall(mmap(NULL, memorySize, PROT_READ | PROT_WRITE,
                                MAP_SHARED, m_sharedMemoryFD, 0),
                           MAP_FAILED);

  if (m_profile == Profile::creator) {
    new (Memory()) SyncedObject();
  }
}

template<class T>
SharedObject<T>::SharedObject(int fd) : m_sharedMemoryFD(fd), m_profile(Profile::user) {
  using namespace detail;
  enum { memorySize = sizeof(SyncedObject) };
  m_sharedMemory =
          (char *)ValidateCall(mmap(NULL, memorySize, PROT_READ | PROT_WRITE,
                                    MAP_SHARED, m_sharedMemoryFD, 0),
                               MAP_FAILED);
}

template <class T> SharedObject<T>::~SharedObject() {
  if (m_profile == Profile::creator) {
    SyncedObject().~SyncedObject();
  }
}

template <class T> typename SharedObject<T>::Handle SharedObject<T>::Acquire() {
  auto syncedObject = Object();
  syncedObject->mutex.lock();
  auto mx = &syncedObject->mutex;
  Handle payload(&syncedObject->object, [mx](T *) { mx->unlock(); });
  return payload;
}

template <class T> char *SharedObject<T>::Memory() { return m_sharedMemory; }

template <class T>
typename SharedObject<T>::SyncedObject *SharedObject<T>::Object() {
  return reinterpret_cast<SyncedObject *>(Memory());
}

template <class T> const char *SharedObject<T>::SharedMemoryName() const {
  return m_sharedMemoryName.c_str();
}

template <class T> const char *SharedObject<T>::SharedMemoryFolder() const {
  return m_sharedMemoryFolder.c_str();
}

template<class T>
int SharedObject<T>::FD() const {
  return m_sharedMemoryFD;
}
