#include "../dojo.h"

using asio::ip::tcp;

Dojo::Net::AsyncTcp::Server::Server(asio::io_context& io_context, short port)
  : acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
{
  DoAccept();
}

void Dojo::Net::AsyncTcp::Server::DoAccept()
{
  acceptor_.async_accept(
    [this](std::error_code ec, tcp::socket socket)
    {
      if (!ec)
      {
        std::make_shared<Receiver::Session>(std::move(socket))->Start();
      }

      DoAccept();
    });
}
