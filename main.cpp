#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

#include <map>
#include <cstdio>
#include <string>
#include <sstream>

using boost::asio::ip::tcp;

boost::shared_ptr<tcp::socket> sock_a;
boost::shared_ptr<tcp::socket> sock_b;
boost::asio::streambuf buf_a;
boost::asio::streambuf buf_b;

double mean_value = 0;

long long last_a = 0;
long long last_b = 0;

std::map<uint64_t, double> values;

void update() {
    while (!values.empty()) {

    }
}

void insert(uint64_t t, double difference) {
    mean_value = (mean_value * values.size() + difference) / (values.size() + 1);
    values.insert(std::make_pair(t, difference));
}

void erase(uint64_t t, double difference) {
    if (values.empty()) {
        return;
    }

    mean_value = (mean_value * values.size() - difference) / (values.size() - 1);
    values.erase(t);
}





template <typename F>
void handle_read(const boost::system::error_code& ec, tcp::socket& sock, boost::asio::streambuf& buf, F f) {
    if (!ec) {
        std::string line;
        std::getline(std::istream(&buf), line);

        std::istringstream iss(line);

        char c;
        uint64_t t;
        long long v;
        iss >> c >> t >> v;
        std::cout << c << " " << t << " " << v << std::endl;

        insert(t, v);

        std::cout << mean_value << " " << values.size() << std::endl;
    } else {
        // TODO: Handle disconnect
    }

    boost::asio::async_read_until(sock, buf, '\n', boost::bind(f, _1));
}

void handle_read_a(const boost::system::error_code& ec) {
    handle_read(ec, *sock_a, buf_a, handle_read_a);
}

void handle_read_b(const boost::system::error_code& ec) {
    handle_read(ec, *sock_b, buf_b, handle_read_b);
}



int interval = 0;

int main(int argc, char* argv[]) {
    boost::asio::io_service io_service;
    tcp::resolver resolver(io_service);

    sock_a.reset(new tcp::socket(io_service));
    sock_b.reset(new tcp::socket(io_service));

    tcp::resolver::query query_a(tcp::v4(), "45.55.178.54", "12301");
    tcp::resolver::query query_b(tcp::v4(), "45.55.178.54", "12302");

    boost::asio::connect(*sock_a, resolver.resolve(query_a));
    boost::asio::connect(*sock_b, resolver.resolve(query_b));

    boost::asio::async_read_until(*sock_a, buf_a, '\n', boost::bind(&handle_read_a, _1));
    boost::asio::async_read_until(*sock_b, buf_b, '\n', boost::bind(&handle_read_b, _1));

    io_service.run();

    return 0;
}
