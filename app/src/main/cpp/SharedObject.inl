#pragma once

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>

template <class T>
SharedObject<T>::SharedObject(std::string objectName, Profile profile,
                              std::string sharedMemoryFolder)
    : m_sharedMemoryName(objectName + "_memory"),
      m_sharedMemoryFolder(sharedMemoryFolder), m_profile(profile) {
  namespace bip = boost::interprocess;

  enum { memorySize = sizeof(SyncedObject) };

  if (m_profile == Profile::creator) {
    boost::interprocess::permissions permissions;
    permissions.set_unrestricted();
    m_sharedMemory = std::make_unique<bip::shared_memory_object>(
        bip::open_or_create, SharedMemoryName(), bip::read_write,
        /*SharedMemoryFolder(),*/ permissions);
    m_sharedMemory->truncate(memorySize);
  } else {
    m_sharedMemory = std::make_unique<bip::shared_memory_object>(
        bip::open_only, SharedMemoryName(),
        bip::read_write /*SharedMemoryFolder()*/);
  }

  m_memoryRegion = std::make_unique<bip::mapped_region>(
      *m_sharedMemory.get(), bip::read_write, 0, memorySize);

  if (m_profile == Profile::creator) {
    new (Memory()) SyncedObject();
  }
}

template <class T> SharedObject<T>::~SharedObject() {
  namespace bip = boost::interprocess;
  if (m_profile == Profile::creator) {
    SyncedObject().~SyncedObject();
    bip::shared_memory_object::remove(SharedMemoryName());
  }
}

template <class T> typename SharedObject<T>::Handle SharedObject<T>::Acquire() {
  namespace bip = boost::interprocess;
  auto syncedObject = Object();
  syncedObject->mutex.lock();
  Handle payload(&syncedObject->object,
                 [mx = &syncedObject->mutex](T *) { mx->unlock(); });
  return payload;
}

template <class T> char *SharedObject<T>::Memory() {
  return (char *)m_memoryRegion->get_address();
}

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
