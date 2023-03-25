#include "../dojo.h"

using asio::ip::tcp;

Dojo::Net::Receiver::Session::Session(tcp::socket socket)
  : socket_(std::move(socket))
{
}

void Dojo::Net::Receiver::Session::Start()
{
  receiver_header_read = false;
  receiver_start_read = false;
  DoReadHeader();
}

void Dojo::Net::Receiver::Session::DoReadHeader()
{
  auto self(shared_from_this());

  socket_.async_read_some(asio::buffer(data_, HEADER_LEN),
    [this, self](std::error_code ec, std::size_t length)
    {
      if (!ec)
      {
        if (length == 0)
        {
          //INFO_LOG(NETWORK, "Client disconnected");
          socket_.close();
        }

        if (length == HEADER_LEN)
        {
          u32 size = Message::GetSize((unsigned char*)data_);
          u32 seq = Message::GetSeq((unsigned char*)data_);
          u32 cmd = Message::GetCmd((unsigned char*)data_);

          working_size = size;
          working_cmd = cmd;

          memcpy(message, data_, HEADER_LEN);

          DoReadBody();
        }

      }
    });
}

void Dojo::Net::Receiver::Session::DoReadBody()
{
  auto self(shared_from_this());

  socket_.async_read_some(asio::buffer(&data_[HEADER_LEN], working_size),
    [this, self](std::error_code ec, std::size_t length)
    {
      if (!ec)
      {
        if (working_size > 0 && length == working_size)
        {
          const char* body = data_ + HEADER_LEN;
          int offset = 0;

          Message::ProcessBody(working_cmd, working_size, body, &offset);
        }

        DoReadHeader();
      }
    });
}

void Dojo::Net::Receiver::Session::DoWrite(std::size_t length)
{
  auto self(shared_from_this());
  asio::async_write(socket_, asio::buffer(data_, length),
    [this, self](std::error_code ec, std::size_t /*length*/)
    {
      if (!ec)
      {
        //INFO_LOG(NETWORK, "Message Sent: %s", data_);
        DoReadHeader();
      }
    });
}

void Dojo::Net::Receiver::ServerThread()
{
  try
  {
    asio::io_context io_context;
    AsyncTcp::Server s(io_context, std::stoi(Net::transmit_port));
    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }
}

void Dojo::Net::Receiver::ClientThread()
{
  try
  {
    asio::io_context io_context;
    tcp::resolver r(io_context);
    AsyncTcp::Client c(io_context);

    c.Start(r.resolve(host_server, host_port));

    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }
}

std::vector<u8> Dojo::Net::Receiver::CreateSpectateRequest()
{
    Message::Writer spectate_request;
    spectate_request.AppendHeader(1, 4);
    spectate_request.AppendString("Quark");
    spectate_request.AppendString("MatchCode");

    std::vector<u8> message = spectate_request.Msg();

    return message;
}

void Dojo::Net::Receiver::StartThread()
{
  if (!Dojo::Session::receiver_started)
  {
    if (hosting)
    {
      std::thread t5(&Dojo::Net::Receiver::ServerThread);
      t5.detach();
    }
    else
    {
      std::thread t5(&Dojo::Net::Receiver::ClientThread);
      t5.detach();
    }

    Dojo::Session::receiver_started = true;
  }
}

