#pragma once
#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>

#include <malloc.h>

#include "../deps/asio.hpp"
#include "receiver.h"
#include "transmitter.h"
#include "async_tcp.h"
#include "tcp.h"
#include "udp.h"

using asio::ip::tcp;
using asio::ip::udp;

namespace Dojo::Net {
  inline bool hosting = false;

  inline std::string host_server;
  inline std::string host_port;
  inline std::string transmit_server;
  inline std::string transmit_port;
  inline std::string transport;

} // namespace Dojo::Net

