#pragma once
#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>

#include "../deps/asio.hpp"

using asio::ip::tcp;
using std::placeholders::_1;
using std::placeholders::_2;

namespace Dojo::Net::AsyncTcp {

class Client
{
public:
  Client(asio::io_context& io_context);
  void Start(tcp::resolver::results_type endpoints);
  void Stop();

private:
  void StartConnect(tcp::resolver::results_type::iterator endpoint_iter);
  void HandleConnect(const asio::error_code& ec, tcp::resolver::results_type::iterator endpoint_iter);

  bool stopped_;
  tcp::resolver::results_type endpoints_;
  tcp::socket socket_;
};

class Server
{
public:
	Server();
	Server(asio::io_context& io_context, short port);

private:
	void DoAccept();

	tcp::acceptor acceptor_;
};

} // namespace Dojo::Net

