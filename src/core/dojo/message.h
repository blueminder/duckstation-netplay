#define GAME_START 1
#define GAME_BUFFER 2
#define SPECTATE_START 3
#define SPECTATE_REQUEST 4
#define PLAYER_INFO 5
#define GAME_END 6

#define HEADER_SIZE 12
#define FRAME_BATCH 120

namespace Dojo::Message {
  u32 GetSize(const u8* buffer);
  u32 GetSeq(const u8* buffer);
  u32 GetCmd(const u8* buffer);

  u32 ReadInt(const char* buffer, int* offset);
  std::string ReadString(const char* buffer, int* offset);
  std::string ReadContinuousData(const char* buffer, int* offset, u32 len);
  std::vector<std::string> ReadPlayerInfo(const char* buffer, int* offset);

  void ProcessBody(u32 cmd, u32 body_size, const char* buffer, int* offset);

  std::vector<std::string> SplitString(const std::string input, const char& delimiter);

  class Writer
  {
  private:
    std::vector<u8> message;
    u32 size;

  public:
    Writer();

    void AppendHeader(u32 _sequence, u32 _command);

    u32 UpdateSize();
    u32 GetSize();

    void AppendInt(u32 value);
    void AppendString(std::string value);
    void AppendData(const char* value, u32 size);

    // append int by divisible data size after header before calling
    void AppendContinuousData(const char* value, u32 size);
    std::vector<u8> Msg();
  }; // class Writer

}  // namespace Dojo::Message

