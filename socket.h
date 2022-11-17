#include "root_certiticates.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
using namespace std::chrono_literals;

class Socket
{

public:
    Socket(std::string host, std::string target) : _host(host), _target(target) {}

    void Start()
    {
        load_root_certificates(_ctx);
        auto const results = _resolver.resolve(_host, "443");

        auto ep = net::connect(get_lowest_layer(_ws), results);

        if (!SSL_set_tlsext_host_name(_ws.next_layer().native_handle(), _host.c_str()))
            throw beast::system_error(
                beast::error_code(
                    static_cast<int>(::ERR_get_error()),
                    net::error::get_ssl_category()),
                "Failed to set SNI Hostname");

        _host += ':' + std::to_string(ep.port());

        _ws.next_layer().handshake(ssl::stream_base::client);

        // Set a decorator to change the User-Agent of the handshake
        _ws.set_option(websocket::stream_base::decorator(
            [](websocket::request_type &req)
            {
                req.set(http::field::user_agent,
                        std::string(BOOST_BEAST_VERSION_STRING) +
                            " websocket-client-coro");
            }));

        // Perform the websocket handshake
        _ws.handshake(_host, _target);
    }

    void Write(const std::string &text)
    {
        _ws.write(net::buffer(text));
    }

    void Read()
    {
        _ws.read(_buffer);
    }

    std::string Message()
    {
        return beast::buffers_to_string(_buffer.data());
    }

    void ClearBuffer()
    {
        _buffer.clear();
    }

    void Close()
    {
        _ws.close(websocket::close_code::none);
    }

private:
    net::io_context _ioc;
    ssl::context _ctx{ssl::context::tlsv12_client};
    tcp::resolver _resolver{_ioc};
    websocket::stream<beast::ssl_stream<tcp::socket>> _ws{_ioc, _ctx};
    beast::flat_buffer _buffer;

    std::string _host;
    std::string _target;
};