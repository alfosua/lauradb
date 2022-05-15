#include "parsing/core.hpp"
#include "parsing/sql.hpp"
#include <boost/asio.hpp>
#include <concepts>
#include <fmt/core.h>
#include <functional>

using boost::asio::ip::tcp;
using std::string;

string read(tcp::socket &socket);
void send(tcp::socket &socket, const string &message);
void serve();

template <typename F, typename I, typename O>
concept action = requires(F &&f, I &&i) {
    { std::forward<F>(f)(std::forward<I>(i)) }
    ->std::convertible_to<O>;
};

template <typename I, typename O, action<I, O> A>
std::vector<O> map(const std::vector<I> &values, const A &action) {
    auto res = std::vector<O>();

    for (auto v : values)
        res.push_back(action(v));

    return res;
}

string int_to_sqr_text(int v) { return std::to_string(v * v); }

int main() {
    // auto pow = 5;
    auto source = std::vector<int>{1, 2, 3, 4, 5};
    // auto sqres = map(source, int_to_sqr_text);
    // auto cubes = map<int, std::string>(
    //     source, [](int v) { return std::to_string(v * v * v); });
    // auto bypow = map<int, std::string>(
    //     source, [&pow](int v) { return std::to_string(std::pow(v, pow)); });

    // for (auto sqr : sqres)
    //     fmt::print("Square: {}\n", sqr);
    // for (auto cub : cubes)
    //     fmt::print("Cube: {}\n", cub);
    // for (auto pow : bypow)
    //     fmt::print("Custom: {}\n", pow);
}

void serve() {
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
