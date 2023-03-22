namespace Dojo::Net::Tcp {
  void Server(asio::io_context& io_context, unsigned short port);
  void ServerSession(tcp::socket socket);

  void SessionLoop(tcp::socket& socket_);

  void ServerThread();
  void ClientThread();

  void StartThread();
}

