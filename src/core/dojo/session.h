#pragma once
#include <cinttypes>
#include <deque>
#include <map>
#include <set>

#include "core/pad.h"
#include "core/system.h"
#include "core/settings.h"
#include "core/host.h"

namespace Dojo::Session {
  inline bool enabled = false;
  inline bool record = false;
  inline bool replay = false;
  inline bool receive = false;

  inline u32 index;
  inline u32 delay;

  inline std::string replay_filename = "replays/game_timestamp.duckr";

  void SetReplayFilename(std::string filename);

  inline std::map<u32, std::string> net_frames[NUM_CONTROLLER_AND_CARD_PORTS];

  inline std::map<u32, u16> net_inputs[NUM_CONTROLLER_AND_CARD_PORTS];
  inline std::array<u16, NUM_CONTROLLER_AND_CARD_PORTS> last_held_input;

  inline std::set<int> active_players;
  void UpdateActivePlayers();

  static u8 target_slot;
  inline bool players_swapped = false;

  u16 GetLastHeldInput(u8 slot);
  void SetLastHeldInput(u8 slot, u16 button_state);

  void Init(std::string game_title, bool record, bool replay);
  void FrameAction();
  void ControllerFrameAction(u8 slot);

  void AddNetFrame(const char* received_data);

  inline std::string current_frame;
  inline std::string target_frame;

  inline bool receiver_started = false;
  inline bool receiver_ended = false;

  inline bool session_ended = false;
  inline u32 frame_timeout = 0;

  void FillStartFrames(u32 fill_delay, int total_players);

  inline std::deque<std::string> transmission_frames;
  inline std::deque<std::string> versus_frames;
  inline volatile bool disconnect_toggle = false;
  inline volatile bool disconnect_sent = false;

  inline bool manual_player_assignment = false;
  inline u32 manual_player;

  inline u32 last_consecutive_common_frame;

} // namespace Dojo::Session

