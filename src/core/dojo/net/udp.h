namespace Dojo::Net::Udp {
inline u32 num_back_frames;
inline u32 num_packets;

void Server(asio::io_context& io_context, unsigned short port);

void ServerThread();
void ClientThread();

void StartThread();

} // namespace Dojo::Net::Udp

