#include "../dojo.h"

void Dojo::Net::Transmitter::StartThread()
{
  if (transmitter_started)
    return;

  if (enabled)
  {
    std::thread t4(&Dojo::Net::Transmitter::ClientThread);
    t4.detach();

    transmitter_started = true;
  }
}

void Dojo::Net::Transmitter::ClientThread()
{
  while (Dojo::Session::net_inputs[1].size() < 60);

  try
  {
    asio::io_context io_context;

    tcp::resolver resolver(io_context);
    tcp::resolver::results_type endpoints =
      resolver.resolve(Net::transmit_server, Net::transmit_port);

    tcp::socket socket(io_context);
    asio::connect(socket, endpoints);

    transmitter_started = true;
    std::string current_frame;
    volatile bool transmission_in_progress;

    Message::Writer spectate_start;

    spectate_start.AppendHeader(1, 3);

    spectate_start.AppendInt(1);
    spectate_start.AppendString("Game");
    spectate_start.AppendString("Player");
    spectate_start.AppendString("Opponent");

    spectate_start.AppendString("Quark");
    spectate_start.AppendString("MatchCode");

    std::vector<unsigned char> message = spectate_start.Msg();
    asio::write(socket, asio::buffer(message));

    std::cout << "Transmission Started" << std::endl;

    unsigned int sent_frame_count = 0;

    Message::Writer frame_msg;
    frame_msg.AppendHeader(0, GAME_BUFFER);

    // start with individual frame size
    // (body_size % data_size == 0)
    frame_msg.AppendInt(FRAME_SIZE);

    for (;;)
    {
      transmission_in_progress = !Dojo::Session::transmission_frames.empty() && !transmitter_ended;

      if (transmission_in_progress)
      {
        auto lock = std::unique_lock<std::mutex>(Dojo::Session::tx_guard);

        current_frame = Dojo::Session::transmission_frames.front();
        frame_msg.AppendContinuousData(current_frame.data(), FRAME_SIZE);
        Dojo::Session::transmission_frames.pop_front();

        lock.unlock();

        sent_frame_count++;

        // send packet every 60 frames
        if (sent_frame_count % FRAME_BATCH == 0)
        {
          message = frame_msg.Msg();
          asio::write(socket, asio::buffer(message));

          frame_msg = Message::Writer();
          frame_msg.AppendHeader(sent_frame_count + 1, GAME_BUFFER);

          // start with individual frame size
          // (body_size % data_size == 0)
          frame_msg.AppendInt(FRAME_SIZE);
        }
      }

      if (transmitter_ended ||
        (Dojo::Session::disconnect_toggle && !transmission_in_progress))
      {
        // send remaining frames
        if (sent_frame_count % FRAME_BATCH > 0)
        {
          message = frame_msg.Msg();
          asio::write(socket, asio::buffer(message));
        }

        Message::Writer disconnect_msg;
        disconnect_msg.AppendHeader(sent_frame_count + 1, GAME_BUFFER);
        disconnect_msg.AppendData("000000000000", FRAME_SIZE);
        asio::write(socket, asio::buffer(disconnect_msg.Msg()));
        std::cout << "Transmission Ended" << std::endl;
        break;
      }
    }
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }
}
