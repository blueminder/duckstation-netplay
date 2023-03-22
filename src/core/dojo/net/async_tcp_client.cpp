#include "../dojo.h"

using asio::ip::tcp;

Dojo::Net::AsyncTcp::Client::Client(asio::io_context& io_context)
  : socket_(io_context)
{
}

void Dojo::Net::AsyncTcp::Client::Start(tcp::resolver::results_type endpoints)
{
  endpoints_ = endpoints;
  StartConnect(endpoints_.begin());
}

void Dojo::Net::AsyncTcp::Client::Stop()
{
  stopped_ = true;
  std::error_code ignored_error;
  socket_.close(ignored_error);
}

void Dojo::Net::AsyncTcp::Client::StartConnect(tcp::resolver::results_type::iterator endpoint_iter)
{
  if (endpoint_iter != endpoints_.end())
  {
    std::cout << "Trying " << endpoint_iter->endpoint() << "..." << std::endl;

    socket_.async_connect(endpoint_iter->endpoint(),
      std::bind(&Client::HandleConnect, this, _1, endpoint_iter));
  }
  else
  {
    Stop();
  }
}

void Dojo::Net::AsyncTcp::Client::HandleConnect(const asio::error_code& ec,
    tcp::resolver::results_type::iterator endpoint_iter)
{
  if (stopped_)
    return;

  if (!socket_.is_open())
  {
    std::cout << "Connection timed out." << std::endl;
    StartConnect(++endpoint_iter);
  }

  else if (ec)
  {
    std::cout << "Connection error: " << ec.message() << std::endl;

    socket_.close();
    StartConnect(++endpoint_iter);
  }
  else
  {
    std::cout << "Connected to " << endpoint_iter->endpoint() << std::endl;

    std::vector<u8> message = Receiver::CreateSpectateRequest();
    asio::write(socket_, asio::buffer(message));

    std::make_shared<Receiver::Session>(std::move(socket_))->Start();
  }
}

