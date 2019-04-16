#pragma once

template <class T>
SharedObject<T>::SharedObject(Profile profile)
        : m_profile(profile), m_ipc(profile == Profile::user ? IPC(IPC::Client()) : IPC(IPC::Server(), [this](std::string_view message)
{
  return SharedObject::MyObject(this);
})) {

}

template <class T> SharedObject<T>::~SharedObject() {
}

template <class T> typename SharedObject<T>::Handle SharedObject<T>::Acquire() {
  if(m_profile == Profile::user)
  {
    m_ipc.Send(0, [this](std::string_view serializedReturnValue) {
      memcpy(&m_object, serializedReturnValue.data(), serializedReturnValue.size());
    });
  }
  return Handle(&m_object, [](T*){});
}
