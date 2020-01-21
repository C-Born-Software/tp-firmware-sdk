//------------------------------------------------------------------------------
// Copyright (c) WAGO Kontakttechnik GmbH & Co. KG
//
// PROPRIETARY RIGHTS are involved in the subject matter of this material. All
// manufacturing, reproduction, use and sales rights pertaining to this
// subject matter are governed by the license agreement. The recipient of this
// software implicitly accepts the terms of the license.
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file     Socket.h
///
///  \brief    <short description of the file contents>
///
///  \author   <author> : WAGO Kontakttechnik GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef PFCSPECIFIC_SRC_ALL_LOGICALDEVICE_SOCKET_H_
#define PFCSPECIFIC_SRC_ALL_LOGICALDEVICE_SOCKET_H_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <sys/socket.h>
#include <system_error>
#include <unistd.h>
//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

/**
 * Wrapper of socket file descriptor that complies to RAII concept.
 */
class Socket {
 public:

  Socket(int __domain, int __type, int __protocol) {
    s_ = socket(__domain, __type, __protocol);
    if (s_ < 0) {
      throw ::std::system_error(errno, ::std::system_category(), "failed to open socket!");
    }
  }

  Socket(Socket&& other)
      : s_ { other.s_ } {
    other.s_ = -1;
  }

  ~Socket() {
    if (s_ >= 0)
      close(s_);
  }

  /**
   * Implicit conversion to 'int' for easy use in socket API.
   */
  operator int() const {
    return s_;
  }

 private:
  int s_;
};

#endif /* PFCSPECIFIC_SRC_ALL_LOGICALDEVICE_SOCKET_H_ */
//---- End of source file ------------------------------------------------------

