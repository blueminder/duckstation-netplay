#pragma once
#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>

#include "../deps/asio.hpp"

using asio::ip::tcp;

namespace Dojo::Net::Receiver {
  inline bool receiver_header_read = false;
  inline bool receiver_start_read = false;
  inline bool receiver_ended = false;

  class Session
  	: public std::enable_shared_from_this<Session>
  {
  public:
  	Session(tcp::socket socket);
  
  	void Start();
  
  private:
  	void DoReadHeader();
  	void DoReadBody();
  	void DoWrite(std::size_t length);
  
  	tcp::socket socket_;
  	enum { max_length = 1024 };
  	char data_[max_length];
  	char message[max_length];
  
  	u32 working_size;
  	u32 working_seq;
  	u32 working_cmd;
  };

  void ServerThread();
  void ClientThread();

  std::vector<u8> CreateSpectateRequest();

  void StartThread();
} // namespace Dojo::Net::Receiver

