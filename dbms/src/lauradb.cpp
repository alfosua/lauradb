#include "parsing/core.hpp"
#include <boost/asio.hpp>
#include <fmt/core.h>

using boost::asio::ip::tcp;
using std::string;

string read(tcp::socket &socket);
void send(tcp::socket &socket, const string &message);

int main() {
    boost::asio::io_service io_service;
    tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), 1234));
    tcp::socket socket(io_service);

    acceptor.accept(socket);

    fmt::print("Welcome to LauraDB!\n");
    fmt::print("Listening...\n");

    string request = read(socket);
    fmt::print("Request: {}\n", request);

    fmt::print("Responding to client...\n");
    send(socket,
         "Welcome to this LauraDB instance! But sorry, I cannnot handle "
         "anything yet...");
    fmt::print("Request Finished.\n");
}

string read(tcp::socket &socket) {
    boost::asio::streambuf buf;
    boost::asio::read_until(socket, buf, "\n");
    string data = boost::asio::buffer_cast<const char *>(buf.data());
    return data;
}

void send(tcp::socket &socket, const string &message) {
    const string final_message = message + "\n";
    boost::asio::write(socket, boost::asio::buffer(final_message));
}
