/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#ifndef _SYNCTEST_H
#define _SYNCTEST_H

#include "../ring_buffer.h"
#include "../sync.h"
#include "../types.h"
#include "backend.h"

class SyncTestBackend : public GGPOSession
{
public:
  SyncTestBackend(GGPOSessionCallbacks* cb, char* gamename, int frames, int num_players);
  virtual ~SyncTestBackend();

  virtual GGPOErrorCode DoPoll();
  virtual GGPOErrorCode AddPlayer(GGPOPlayer* player, GGPOPlayerHandle* handle);
  virtual GGPOErrorCode AddLocalInput(GGPOPlayerHandle player, void* values, int size);
  virtual GGPOErrorCode SyncInput(void* values, int size, int* disconnect_flags);
  virtual GGPOErrorCode IncrementFrame(uint16_t checksum);
  virtual GGPOErrorCode Logv(char* fmt, va_list list);
  virtual GGPOErrorCode DisconnectPlayer(GGPOPlayerHandle handle) { return GGPO_OK; }
  virtual GGPOErrorCode Chat(const char* text) override { return GGPO_ERRORCODE_UNSUPPORTED; }
  virtual GGPOErrorCode CurrentFrame(int& current) override;
  virtual GGPOErrorCode ConfirmedFrame(int& confirmed) override;

protected:
  struct SavedInfo
  {
    int frame;
    int checksum;
    char* buf;
    int cbuf;
    GameInput input;
  };

  void RaiseSyncError(const char* fmt, ...);
  void BeginLog(int saving);
  void EndLog();
  void LogSaveStates(SavedInfo& info);

protected:
  GGPOSessionCallbacks _callbacks;
  Sync _sync;
  int _num_players;
  int _check_distance;
  int _last_verified;
  bool _rollingback;
  bool _running;
  FILE* _logfp;
  char _game[128];

  GameInput _current_input;
  GameInput _last_input;
  RingBuffer<SavedInfo, 32> _saved_frames;
};

#endif
