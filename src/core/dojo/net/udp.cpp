#include "../dojo.h"

using asio::ip::udp;

void Dojo::Net::Udp::Server(asio::io_context& io_context, unsigned short port)
{
  udp::socket socket_(io_context, udp::endpoint(udp::v4(), port));

  Message::Writer frame_msg;
  u32 size = 0;
  u32 seq = 0;
  u32 cmd = 0;

  bool header_read = false;

  for (;;)
  {
    char data_[2048] = {0};

    udp::endpoint sender_endpoint;
    size_t length = socket_.receive_from(asio::buffer(data_, 2048), sender_endpoint);

    frame_msg = Message::Writer();
    frame_msg.AppendHeader(0, GAME_BUFFER);

    // start with individual frame size
    // (body_size % data_size == 0)
    frame_msg.AppendInt(FRAME_SIZE);

    while(!Dojo::Session::versus_frames.empty())
    {
        std::string current_frame = Dojo::Session::versus_frames.front();
        frame_msg.AppendContinuousData(current_frame.data(), FRAME_SIZE);

        auto current_frame_number = Frame::GetEffectiveFrameNumber((u8*)current_frame.data());
        auto current_frame_player = Frame::GetPlayer((u8*)current_frame.data());
        auto current_frame_delay = Frame::GetDelay((u8*)current_frame.data());

        for (int i = 0; i < num_back_frames; i++)
        {
          std::string back_frame = Frame::Create(current_frame_number - i, current_frame_player, current_frame_delay,
                        Dojo::Session::net_inputs[current_frame_player][current_frame_number - i]);
          frame_msg.AppendContinuousData(back_frame.data(), FRAME_SIZE);
        }

        Dojo::Session::versus_frames.pop_front();
    }

    auto message = frame_msg.Msg();

    asio::error_code ignored_error;
    for (int i = 0; i < num_packets; i++)
    {
      socket_.send_to(asio::buffer(message), sender_endpoint, 0, ignored_error);
    }

    if (length >= HEADER_LEN)
    {
      size = Message::GetSize((unsigned char*)data_);
      seq = Message::GetSeq((unsigned char*)data_);
      cmd = Message::GetCmd((unsigned char*)data_);

      header_read = true;
    }

    //if (size > 0)
    if (header_read == true)
    {
      int offset = 0;
      Message::ProcessBody(cmd, size, &data_[HEADER_LEN], &offset);
      header_read = false;
    }

    if (Dojo::Session::disconnect_toggle)
    {
      Message::Writer disconnect_msg;
      disconnect_msg.AppendHeader(0, GAME_END);
      disconnect_msg.AppendData("000000000000", FRAME_SIZE);

      for (int i = 0; i < 2; i++)
      {
        socket_.send_to(asio::buffer(disconnect_msg.Msg(), disconnect_msg.GetSize()), sender_endpoint);
      }
      Dojo::Session::disconnect_sent = true;
      socket_.close();
      std::cout << "Session Ended" << std::endl;
      break;
    }
  }
}

void Dojo::Net::Udp::ServerThread()
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

void Dojo::Net::Udp::ClientThread()
{
  try
  {
    asio::io_context io_context;

    udp::resolver resolver(io_context);
    udp::resolver::results_type endpoints =
      resolver.resolve(Net::host_server, Net::host_port);

    udp::socket socket_(io_context);
    socket_.open(udp::v4());
    //asio::connect(socket_, endpoints);

    //transmitter_started = true;
    /*
    std::string current_frame;

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

          auto current_frame_number = Frame::GetFrameNumber((u8*)current_frame.data());
          auto current_frame_player = Frame::GetPlayer((u8*)current_frame.data());
          auto current_frame_delay = Frame::GetDelay((u8*)current_frame.data());

          for (int i = 0; i < num_back_frames; i++)
          {
            std::string back_frame = Frame::Create(current_frame_number - i, current_frame_player, current_frame_delay,
                          Dojo::Session::net_inputs[current_frame_player][current_frame_number - i]);
            frame_msg.AppendContinuousData(back_frame.data(), FRAME_SIZE);
          }

          Dojo::Session::versus_frames.pop_front();
      }

      auto message = frame_msg.Msg();
      for (int i = 0; i < num_packets; i++)
      {
        socket_.send_to(asio::buffer(message), *endpoints.begin());
      }

      udp::endpoint sender_endpoint;
      size_t length = socket_.receive_from(asio::buffer(data_, 2048), sender_endpoint);
      if (length >= HEADER_LEN)
      {
        size = Message::GetSize((unsigned char*)data_);
        seq = Message::GetSeq((unsigned char*)data_);
        cmd = Message::GetCmd((unsigned char*)data_);

        header_read = true;
      }

      //if (size > 0)
      if (header_read == true)
      {
        int offset = 0;
        Message::ProcessBody(cmd, size, &data_[HEADER_LEN], &offset);
        header_read = false;
      }

      if (Dojo::Session::disconnect_toggle)
      {
        Message::Writer disconnect_msg;
        disconnect_msg.AppendHeader(0, GAME_END);
        disconnect_msg.AppendData("000000000000", FRAME_SIZE);

        for (int i = 0; i < num_packets; i++)
        {
          socket_.send_to(asio::buffer(disconnect_msg.Msg(), disconnect_msg.GetSize()), *endpoints.begin());
        }
        Dojo::Session::disconnect_sent = true;
        socket_.close();
        std::cout << "Session Ended" << std::endl;
        break;
      }
    }
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }
}

void Dojo::Net::Udp::StartThread()
{
  if (!Dojo::Session::receiver_started)
  {
    if (Net::hosting)
    {
      std::thread t5(&Dojo::Net::Udp::ServerThread);
      t5.detach();
    }
    else
    {
      std::thread t5(&Dojo::Net::Udp::ClientThread);
      t5.detach();
    }

    Dojo::Session::receiver_started = true;
  }
}

