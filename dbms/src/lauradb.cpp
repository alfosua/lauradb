#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;
using std::string;

int main()
{
    boost::asio::io_service io_service;
    tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), 1234));
    tcp::socket socket(io_service);

    acceptor.accept(socket);

    std::cout << "Welcome to LauraDB!" << std::endl;

    std::cout << "Listening..." << std::endl;

    string request = read(socket);
    std::cout << "Request: " << request << std::endl;

    std::cout << "Responding to client...";
    send(socket, "Welcome to this LauraDB instance! But sorry, I cannnot handle anything yet...");
    std::cout << "Request Finished.";
}

string read(tcp::socket &socket)
{
    boost::asio::streambuf buf;
    boost::asio::read_until(socket, buf, "\n");
    string data = boost::asio::buffer_cast<const char *>(buf.data());
    return data;
}

void send(tcp::socket &socket, const string &message)
{
    const string final_message = message + "\n";
    boost::asio::write(socket, boost::asio::buffer(final_message));
}
