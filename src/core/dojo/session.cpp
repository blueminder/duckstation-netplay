#include "dojo.h"

void Dojo::Session::FrameAction()
{
  if (disconnect_toggle && disconnect_sent)
  {
    Host::AddOSDMessage("Session Ended", 10.0f);
    System::PauseSystem(true);
  }

  if (active_players.size() == 0)
    UpdateActivePlayers();

  if (!Training::enabled && !replay && (Net::hosting || !Net::host_server.empty()))
  {
    while (!disconnect_toggle &&
           std::any_of(active_players.begin(), active_players.end(),
             [](int i) { return (net_inputs[i].count(index) == 0); }));
  }

  for (u32 i = 0; i < NUM_CONTROLLER_AND_CARD_PORTS; i++)
  {
    const ControllerType type = g_settings.controller_types[i];
    if (type == ControllerType::DigitalController || type == ControllerType::AnalogController)
    {
      ControllerFrameAction(i);
    }
  }

  index++;
}

void Dojo::Session::ControllerFrameAction(u8 slot)
{
  Controller* p = Pad::GetController(slot);

  if (!p)
    return;

  if (receive && Net::Receiver::receiver_header_read)
  {
    while ((net_inputs[slot].count(index) == 0) && !session_ended)
    {
      frame_timeout++;

      if (frame_timeout > 2400)
        session_ended = true;
    }
  }

  if (slot == 0)
  {
    if (replay && net_inputs[slot].count(index + 1) == 0)
      session_ended = true;
  }

  if (replay || receive)
  {
    if (disconnect_frame > 0 && index == disconnect_frame)
      Session::disconnect_toggle = true;

    p->SetButtonStateBits(~net_inputs[slot][index]);
  }
  else
  {
    target_slot = slot;
    if (active_players.size() == 2)
    {
      if (Training::enabled && players_swapped)
        target_slot = (u8)(slot == 0 ? 1 : 0);
    }

    if (!Training::enabled && !replay && !receive && (Net::hosting || !Net::host_server.empty()))
    {
      if (slot == 0)
      {
        if (manual_player_assignment)
          target_slot = manual_player;
        else
          target_slot = Net::hosting ? 0 : 1;
      }
    }

    if (net_inputs[target_slot].count(index + delay) == 0)
    {
      target_frame = Frame::Create(index, target_slot, delay, last_held_input[slot]);
      AddNetFrame(target_frame.data());
      versus_frames.push_back(target_frame);
    }

    if (receive && index > net_inputs[1].size() - 2)
    {
      Host::RunOnCPUThread([]() { System::PauseSystem(true); });
      receiver_buffered = false;
      std::string msg = "Buffering...";
      Host::AddOSDMessage(Host::TranslateStdString("OSDMessage", msg.data()), 10.0f);
    }

    p->SetButtonStateBits(~net_inputs[slot][index]);

    current_frame = net_frames[slot][index];

    if (record)
      Replay::AppendFrameToFile(current_frame);

    if (Net::Transmitter::enabled)
    {
      auto lock = std::unique_lock<std::mutex>(tx_guard);
      transmission_frames.push_back(current_frame);
      lock.unlock();
    }

    if (Training::enabled)
      Training::TrainingFrameAction();
  }
}

void Dojo::Session::UpdateActivePlayers()
{
  u32 total = 0;
  for (u32 i = 0; i < NUM_CONTROLLER_AND_CARD_PORTS; i++)
  {
    if (!net_inputs[i].empty())
    {
      active_players.insert(i);
    }
  }

}

u16 Dojo::Session::GetLastHeldInput(u8 slot)
{
  if (slot > last_held_input.size() - 1)
    return 0;

  return last_held_input[slot];
}

void Dojo::Session::SetLastHeldInput(u8 slot, u16 button_state)
{
  if (slot < last_held_input.size())
    last_held_input[slot] = button_state;
}

void Dojo::Session::AddNetFrame(const char* received_data)
{
  const char data[FRAME_SIZE] = { 0 };
  memcpy((void*)data, received_data, FRAME_SIZE);

  // disconnect on blank frame
  if (memcmp(data, "000000000000", FRAME_SIZE) == 0)
    Session::disconnect_frame = net_inputs[1].size() - 2;

  u32 effective_frame_num = Frame::GetEffectiveFrameNumber((u8*)data);
  if (effective_frame_num == 0)
    return;

  u32 frame_player = (u8)data[0];
  if (frame_player > 2)
    return;

  std::string data_to_queue(data, data + FRAME_SIZE);

  if (net_inputs[frame_player].count(effective_frame_num) == 0 ||
    effective_frame_num >= last_consecutive_common_frame)
  {
    net_frames[frame_player].emplace(effective_frame_num, data_to_queue);
    net_inputs[frame_player].emplace(effective_frame_num, Frame::GetDigital((u8*)data));
  }

  if (std::all_of(active_players.begin(), active_players.end(),
    [](int i) { return net_inputs[i].count(index) == 1; }))
  {
    if (effective_frame_num == last_consecutive_common_frame + 1)
      last_consecutive_common_frame++;
  }

  if (receive && !receiver_buffered && System::IsPaused
      && net_inputs[1].size() > 1800 && net_inputs[1].size() > (index + 1800))
  {
    Host::RunOnCPUThread([]() { System::PauseSystem(false); });
    receiver_buffered = true;
  }
}

void Dojo::Session::SetReplayFilename(std::string filename)
{
  replay_filename = filename;
}

void Dojo::Session::FillStartFrames(u32 start_frame, int total_players = 2)
{
  for (int j = 0; j < total_players; j++)
  {
    net_inputs[j][0] = 0;

    std::string f2 = Frame::Create(1, j, 0, 0);
    std::string f3 = Frame::Create(1, j, 1, 0);

    AddNetFrame(f2.data());
    AddNetFrame(f3.data());

    std::string new_frame;
    for (u32 i = 1; i <= start_frame; i++)
    {
      new_frame = Frame::Create(2, j, i, 0);
      AddNetFrame(new_frame.data());
    }
  }
}

void Dojo::Session::Init(std::string game_title, bool p_replay, bool p_training)
{
  replay = p_replay;
  Training::enabled = p_training;

  if (!enabled)
    enabled = enabled || g_settings.dojo.enabled;

  if (!record)
    record = g_settings.dojo.record;

  if (delay == 0)
    delay = g_settings.dojo.delay;

  if (!receive)
    receive = g_settings.dojo.receive;

  Net::Udp::num_back_frames = g_settings.dojo.num_back_frames;
  Net::Udp::num_packets = g_settings.dojo.num_packets;

  index = 0;
  last_consecutive_common_frame = 2;

  // only record live sessions
  if (record && replay)
    record = false;

  if (receive)
  {
    Host::RunOnCPUThread([]() { System::PauseSystem(true); });
    receiver_buffered = false;
    std::string msg = "Buffering...";
    Host::AddOSDMessage(Host::TranslateStdString("OSDMessage", msg.data()), 10.0f);
  }

  for (u32 i = 0; i < NUM_CONTROLLER_AND_CARD_PORTS; i++)
  {
    last_held_input[i] = 0;
  }

  Replay::replay_frame_count = 0;
  if (record)
    Replay::CreateReplayFile(game_title);

  FillStartFrames(delay);

  if (replay)
    Replay::LoadFile(replay_filename);

  if (Net::Transmitter::enabled)
    Net::Transmitter::StartThread();

  if (receive)
    Net::Receiver::StartThread();

  if (Net::transport == "udp")
    Net::Udp::StartThread();
  else
    Net::Tcp::StartThread();
}
