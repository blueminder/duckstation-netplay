#include "dojo.h"

void Dojo::Training::TrainingFrameAction()
{
  if (recording && Frame::GetPlayer((u8*)Session::current_frame.data()) == record_player)
    record_slot[current_record_slot].push_back(Session::current_frame);

  if (!recording && !playing_input &&
    playback_loop && trigger_playback &&
    Session::index > next_playback_frame)
  {
    PlayRecording(current_record_slot);
  }
}

std::string Dojo::Training::ToggleRecording(int slot)
{
  std::ostringstream NoticeStream;
  if (recording)
  {
    recording = false;
    NoticeStream << "Stop Recording Slot " << slot + 1 << " Player " << record_player + 1;
  }
  else
  {
    current_record_slot = slot;
    record_slot[slot].clear();
    recorded_slots.insert(slot);
    recording = true;
    NoticeStream << "Recording Slot " << slot + 1 << " Player " << record_player + 1;
  }
  return NoticeStream.str();
}

std::string Dojo::Training::TogglePlayback(int slot)
{
  std::ostringstream NoticeStream;
  if (playback_loop)
  {
    if (trigger_playback[slot])
    {
      trigger_playback[slot] = false;
      NoticeStream << "Stop Loop Slot " << slot + 1;
    }
    else
    {
      current_record_slot = slot;
      trigger_playback[slot] = true;
      NoticeStream << "Play Loop Slot " << slot + 1;
    }
  }
  else
  {
    current_record_slot = slot;
    NoticeStream << "Play Slot " << slot + 1;
    PlayRecording(slot);
  }
  return NoticeStream.str();
}

std::string Dojo::Training::ToggleRandomPlayback()
{
  if (recorded_slots.empty())
  {
    return "No Input Slots Recorded";
  }
  if (!playing_input)
  {
    auto it = recorded_slots.cbegin();
    srand(time(0));
    int rnd = rand() % recorded_slots.size();
    std::advance(it, rnd);
    current_record_slot = *it;
  }
  std::string notice = TogglePlayback(current_record_slot);
  return notice.substr(0, notice.size() - 2);
}

void Dojo::Training::PlayRecording(int slot)
{
  if (!recording && !playing_input)
  {
    playing_input = true;
    u8 to_add[FRAME_SIZE] = { 0 };
    u32 target_frame = Dojo::Session::index + 1 + Dojo::Session::delay;
    for (std::string frame : record_slot[slot])
    {
      //to_add[0] = (u8)port;
      memcpy(to_add, frame.data(), FRAME_SIZE);
      memcpy(to_add + 2, (u8*)&target_frame, 4);
      Session::AddNetFrame((const char *)to_add);
      target_frame++;
    }
    next_playback_frame = target_frame;
    playing_input = false;
  }
}

std::string Dojo::Training::ToggleLoop()
{
  std::ostringstream NoticeStream;
  if (playback_loop)
  {
    playback_loop = false;
    NoticeStream << "Playback Loop Disabled.";
  }
  else
  {
    playback_loop = true;
    NoticeStream << "Playback Loop Enabled";
  }
  return NoticeStream.str();
}

std::string Dojo::Training::TogglePlayerSwap()
{
  record_player == 0 ?
    record_player = 1 :
    record_player = 0;

  Session::players_swapped = !Session::players_swapped;

  std::ostringstream NoticeStream;
  NoticeStream << "Controlling Player " << record_player + 1;
  return NoticeStream.str();
}

void Dojo::Training::ResetTraining()
{
  Session::players_swapped = false;

  record_player = 0;
  current_record_slot = 0;

  for (int i = 0; i < 3; i++)
  {
    record_slot[i].clear();
  }

  recorded_slots.clear();
}
