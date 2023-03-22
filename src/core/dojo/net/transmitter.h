#pragma once
#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>

#include "../deps/asio.hpp"

using asio::ip::tcp;

namespace Dojo::Net::Transmitter
{
  inline bool enabled = false;
  inline bool transmitter_started = false;
  inline bool transmitter_ended = false;

  void ClientThread();
  void StartThread();
}