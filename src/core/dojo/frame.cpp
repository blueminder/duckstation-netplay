#include "dojo.h"

std::string Dojo::Frame::Create(u32 frame_num, int player, u32 delay, u16 digital)
{
  u8 new_frame[FRAME_SIZE] = { 0 };
  new_frame[0] = player;
  new_frame[1] = delay;

  // enter current frame count in next 4 bytes
  memcpy(new_frame + 2, (u8*)&frame_num, sizeof(u32));

  if (digital != 0)
    memcpy(new_frame + 6, (u8*)&digital, sizeof(u16));

  // net_frame byte indices 8-11 reserved for axis

  u8 ret_frame[FRAME_SIZE] = { 0 };
  memcpy((void*)ret_frame, new_frame, FRAME_SIZE);

  std::string frame_str(ret_frame, ret_frame + FRAME_SIZE);

  return frame_str;
}

u32 Dojo::Frame::GetPlayer(u8* data)
{
  return (u32)data[0];
}

u32 Dojo::Frame::GetDelay(u8* data)
{
  return (u32)data[1];
}

u32 Dojo::Frame::GetFrameNumber(u8* data)
{
  return (*(u32*)(data + 2));
}

u32 Dojo::Frame::GetEffectiveFrameNumber(u8* data)
{
  return GetFrameNumber(data) + GetDelay(data);
}

u16 Dojo::Frame::GetDigital(u8* data)
{
  return (*(u16*)(data + 6));
}
