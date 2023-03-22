#include "../dojo.h"

void Dojo::Net::Tcp::ServerSession(tcp::socket socket_)
{
  try
  {
    std::cout << "Session Started" << std::endl;

    SessionLoop(socket_);
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }
}

void Dojo::Net::Tcp::Server(asio::io_context& io_context, unsigned short port)
{
  tcp::acceptor a(io_context, tcp::endpoint(tcp::v4(), port));
  for (;;)
  {
    std::thread(ServerSession, a.accept()).detach();
  }
}

void Dojo::Net::Tcp::ServerThread()
{
  try
  {
    asio::io_context io_context;
    Server(io_context, std::atoi(Net::host_port.data()));
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }
}

void Dojo::Net::Tcp::SessionLoop(tcp::socket& socket_)
{
  Message::Writer frame_msg;
  u32 size = 0;
  u32 seq = 0;
  u32 cmd = 0;

  bool header_read = false;

  for (;;)
  {
    char data_[2048] = {0};

    frame_msg = Message::Writer();
    frame_msg.AppendHeader(0, GAME_BUFFER);

    // start with individual frame size
    // (body_size % data_size == 0)
    frame_msg.AppendInt(FRAME_SIZE);

    while(!Dojo::Session::versus_frames.empty())
    {
        std::string current_frame = Dojo::Session::versus_frames.front();
        frame_msg.AppendContinuousData(current_frame.data(), FRAME_SIZE);
        Dojo::Session::versus_frames.pop_front();
    }

    auto message = frame_msg.Msg();
    asio::write(socket_, asio::buffer(message));

    asio::error_code error;
    size_t length = socket_.read_some(asio::buffer(data_, HEADER_LEN), error);
    if (length == HEADER_LEN)
    {
      size = Message::GetSize((unsigned char*)data_);
      seq = Message::GetSeq((unsigned char*)data_);
      cmd = Message::GetCmd((unsigned char*)data_);

      header_read = true;
    }

    //if (size > 0)
    if (size > 0 && header_read == true)
    {
      size_t read_size = socket_.read_some(asio::buffer(&data_[HEADER_LEN], size), error);
      if (read_size == size)
      {
        int offset = 0;
        Message::ProcessBody(cmd, size, &data_[HEADER_LEN], &offset);
        header_read = false;
      }
    }

    if (Dojo::Session::disconnect_toggle)
    {
      Message::Writer disconnect_msg;
      disconnect_msg.AppendHeader(0, GAME_END);
      disconnect_msg.AppendData("000000000000", FRAME_SIZE);
      asio::write(socket_, asio::buffer(disconnect_msg.Msg()), error);
      Dojo::Session::disconnect_sent = true;
      socket_.close();
      std::cout << "Session Ended" << std::endl;
      break;
    }

    if (error == asio::error::eof)
        break; // Connection closed cleanly by peer.
    else if (error)
        throw asio::system_error(error); // Some other error.

  }
}

void Dojo::Net::Tcp::ClientThread()
{
  try
  {
    asio::io_context io_context;

    tcp::resolver resolver(io_context);
    tcp::resolver::results_type endpoints =
      resolver.resolve(Net::host_server, Net::host_port);

    tcp::socket socket_(io_context);
    asio::connect(socket_, endpoints);

    //transmitter_started = true;
    std::string current_frame;

    /*
    Message::Writer game_start;

    game_start.AppendHeader(0, GAME_START);

    game_start.AppendInt(1);
    game_start.AppendString("Game");
    game_start.AppendString("Player");
    game_start.AppendString("Opponent");

    game_start.AppendString("Quark");
    game_start.AppendString("MatchCode");

    std::vector<unsigned char> message = game_start.Msg();
    asio::write(socket_, asio::buffer(message));
    */
    std::cout << "Session Started" << std::endl;

    SessionLoop(socket_);
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }
}

void Dojo::Net::Tcp::StartThread()
{
  if (!Dojo::Session::receiver_started)
  {
    if (Net::hosting)
    {
      std::thread t5(&Dojo::Net::Tcp::ServerThread);
      t5.detach();
    }
    else
    {
      std::thread t5(&Dojo::Net::Tcp::ClientThread);
      t5.detach();
    }

    Dojo::Session::receiver_started = true;
  }
}

