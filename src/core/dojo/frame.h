namespace Dojo::Frame {
  // frame data extraction methods
  u32 GetPlayer(u8* data);
  u32 GetDelay(u8* data);
  u32 GetFrameNumber(u8* data);
  u32 GetEffectiveFrameNumber(u8* data);
  u16 GetDigital(u8* data);

  std::string Create(u32 frame_num, int player, u32 delay, u16 digital);
} // namespace Dojo::Frame

