#include <iostream>
#include <boost/asio.hpp>

using namespace boost::asio;
using namespace boost::asio::ip;

class Server {
public:
  Server(io_service& service, short port)
    : acceptor_(service, tcp::endpoint(tcp::v4(), port)),
      socket_(service)
  {
    startAccept();
  }

private:
  void startAccept()
  {
    acceptor_.async_accept(socket_,
      [this](boost::system::error_code ec)
      {
        if (!ec)
        {
          std::cout << "Client connected." << std::endl;
          handleRequest();
        }
        else
        {
          std::cerr << "Error accepting connection: " << ec.message() << std::endl;
        }

        startAccept();
      });
  }

  void handleRequest()
  {
    boost::system::error_code ec;
    streambuf buffer;
    read_until(socket_, buffer, "\r\n\r\n", ec);

    if (!ec)
    {
      std::string message;
      std::istream request_stream(&buffer);
      std::getline(request_stream, message);

      std::cout << "Received request: " << message << std::endl;

      // Send a response
      std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
      write(socket_, buffer(response), ec);

      if (!ec)
      {
        std::cout << "Response sent." << std::endl;
      }
      else
      {
        std::cerr << "Error sending response: " << ec.message() << std::endl;
      }
    }
    else
    {
      std::cerr << "Error reading request: " << ec.message() << std::endl;
    }
  }

  tcp::acceptor acceptor_;
  tcp::socket socket_;
};

int main()
{
  boost::asio::io_service service;
  Server server(service, 8080);
  service.run();

  return 0;
}
