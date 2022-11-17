#include "webServer.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/config.hpp>
#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <boost/filesystem.hpp>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

std::string publicPath = boost::filesystem::canonical("public").string();

// Return a reasonable mime type based on the extension of a file.
beast::string_view mime_type(beast::string_view path)  {
    using beast::iequals;
    auto const ext = [&path] {
        auto const pos = path.rfind(".");
        if (pos == beast::string_view::npos)
            return beast::string_view{};
        return path.substr(pos);
    }();
    if (iequals(ext, ".htm")) return "text/html";
    if (iequals(ext, ".html")) return "text/html";
    if (iequals(ext, ".php")) return "text/html";
    if (iequals(ext, ".css")) return "text/css";
    if (iequals(ext, ".txt")) return "text/plain";
    if (iequals(ext, ".js")) return "application/javascript";
    if (iequals(ext, ".json")) return "application/json";
    if (iequals(ext, ".xml")) return "application/xml";
    if (iequals(ext, ".swf")) return "application/x-shockwave-flash";
    if (iequals(ext, ".flv")) return "video/x-flv";
    if (iequals(ext, ".png")) return "image/png";
    if (iequals(ext, ".jpe")) return "image/jpeg";
    if (iequals(ext, ".jpeg")) return "image/jpeg";
    if (iequals(ext, ".jpg")) return "image/jpeg";
    if (iequals(ext, ".gif")) return "image/gif";
    if (iequals(ext, ".bmp")) return "image/bmp";
    if (iequals(ext, ".ico")) return "image/x-icon";
    if (iequals(ext, ".tiff")) return "image/tiff";
    if (iequals(ext, ".tif")) return "image/tiff";
    if (iequals(ext, ".svg")) return "image/svg+xml";
    if (iequals(ext, ".svgz")) return "image/svg+xml";
    return "application/text";
}

// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
std::string path_cat(beast::string_view base, beast::string_view path) {
    if (base.empty())
        return std::string(path);
    std::string result(base);
#ifdef BOOST_MSVC
    char constexpr path_separator = '\\';
    if(result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
    for(auto& c : result)
        if(c == '/')
            c = path_separator;
#else
    char constexpr path_separator = '/';
    if (result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
#endif
    return result;
}

// This function produces an HTTP response for the given
// request. The type of the response object depends on the
// contents of the request, so the interface requires the
// caller to pass a generic lambda for receiving the response.
template<class Body, class Allocator, class Send>
void handle_request(
        beast::string_view doc_root,
        http::request<Body, http::basic_fields<Allocator>> &&req,
        Send &&send) {
    // Returns a bad request response
    auto const bad_request =
            [&req](beast::string_view why) {
                http::response<http::string_body> res{http::status::bad_request, req.version()};
                res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                res.set(http::field::content_type, "text/html");
                res.keep_alive(req.keep_alive());
                res.body() = std::string(why);
                res.prepare_payload();
                return res;
            };

    // Returns a not found response
    auto const not_found =
            [&req](beast::string_view target) {
                http::response<http::string_body> res{http::status::not_found, req.version()};
                res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                res.set(http::field::content_type, "text/html");
                res.keep_alive(req.keep_alive());
                res.body() = "The resource '" + std::string(target) + "' was not found.";
                res.prepare_payload();
                return res;
            };

    // Returns a server error response
    auto const server_error =
            [&req](/*beast::string_view what*/) {
                http::response<http::string_body> res{http::status::internal_server_error, req.version()};
                res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                //res.set(http::field::content_type, "text/html");
                res.keep_alive(req.keep_alive());
                //res.body() = "An error occurred: '" + std::string(what) + "'";
                res.prepare_payload();
                return res;
            };

    // Make sure we can handle the method
    if (req.method() != http::verb::get)
        return send(bad_request("Unknown HTTP-method"));

    // Request path must be absolute and not contain "..".
    if (req.target().empty() ||
        req.target()[0] != '/' ||
        req.target().find("..") != beast::string_view::npos)
        return send(bad_request("Illegal request-target"));

    // Build the path to the requested file
    // std::string path = path_cat(doc_root, req.target());

    std::string absolutPath = path_cat("", req.target());

    // Static get requests
    auto i = staticGets.find(absolutPath);
    if (i != staticGets.end()) {
        try {
            return i->second(ContextManualStatic {req}, send);
        } catch (const std::exception& e) {
            std::cout << e.what() << std::endl;
            return send(server_error());
        }
    }

    // Static file request
    beast::error_code ec;
    boost::filesystem::path filePath = boost::filesystem::weakly_canonical("public" + absolutPath, ec);

    if (!ec && boost::filesystem::is_regular_file(filePath)) {

        if (!filePath.string().starts_with(publicPath)) return send(bad_request("Illegal request-target"));

        // Attempt to open the file
        http::file_body::value_type body;
        body.open(reinterpret_cast<const char*>(filePath.c_str()), beast::file_mode::scan, ec);

        if (ec) return send(server_error());

        // Cache the size since we need it after the move
        auto const size = body.size();

        // Respond to GET request
        http::response<http::file_body> res{
                std::piecewise_construct,
                std::make_tuple(std::move(body)),
                std::make_tuple(http::status::ok, req.version())};
        //res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, mime_type(absolutPath));
        res.content_length(size);
        res.keep_alive(req.keep_alive());
        return send(std::move(res));
    }

    // Dynamic get request

    // Prevent std::regex segfault with long input
    if (absolutPath.length() >= 160) return send(bad_request("Request-target too long"));

    for(const auto& dynGet : dynamicGets) {
        std::smatch match;
        if (std::regex_match(absolutPath, match, dynGet.first)) {
            try {
                return dynGet.second(ContextManualDynamic {req, match}, send);
            } catch (const std::exception& e) {
                std::cout << e.what() << std::endl;
                return send(server_error());
            }
        }
    }

    return send(not_found(absolutPath));
}

//------------------------------------------------------------------------------

// Report a failure
void fail(beast::error_code ec, char const *what) {
    std::cerr << what << ": " << ec.message() << "\n";
}

// Handles an HTTP server connection
void do_session(
        beast::tcp_stream &stream,
        std::shared_ptr<std::string const> const &doc_root,
        net::yield_context yield) {
    bool close = false;
    beast::error_code ec;

    // This buffer is required to persist across reads
    beast::flat_buffer buffer;

    // This lambda is used to send messages
    send_lambda lambda{stream, close, ec, yield};

    for (;;) {
        // Set the timeout.
        stream.expires_after(std::chrono::seconds(30));

        // Read a request
        http::request<http::string_body> req;
        http::async_read(stream, buffer, req, yield[ec]);
        if (ec == http::error::end_of_stream)
            break;
        if (ec)
            return fail(ec, "read");

        // Send the response
        handle_request(*doc_root, std::move(req), lambda);
        if (ec)
            return fail(ec, "write");
        if (close) {
            // This means we should close the connection, usually because
            // the response indicated the "Connection: close" semantic.
            break;
        }
    }

    // Send a TCP shutdown
    stream.socket().shutdown(tcp::socket::shutdown_send, ec);

    // At this point the connection is closed gracefully
}

//------------------------------------------------------------------------------

// Accepts incoming connections and launches the sessions
void do_listen(
        net::io_context &ioc,
        tcp::endpoint endpoint,
        std::shared_ptr<std::string const> const &doc_root,
        net::yield_context yield) {
    beast::error_code ec;

    // Open the acceptor
    tcp::acceptor acceptor(ioc);
    acceptor.open(endpoint.protocol(), ec);
    if (ec)
        return fail(ec, "open");

    // Allow address reuse
    acceptor.set_option(net::socket_base::reuse_address(true), ec);
    if (ec)
        return fail(ec, "set_option");

    // Bind to the server address
    acceptor.bind(endpoint, ec);
    if (ec)
        return fail(ec, "bind");

    // Start listening for connections
    acceptor.listen(net::socket_base::max_listen_connections, ec);
    if (ec)
        return fail(ec, "listen");

    for (;;) {
        tcp::socket socket(ioc);
        acceptor.async_accept(socket, yield[ec]);
        if (ec)
            fail(ec, "accept");
        else
            boost::asio::spawn(
                    acceptor.get_executor(),
                    std::bind(
                            &do_session,
                            beast::tcp_stream(std::move(socket)),
                            doc_root,
                            std::placeholders::_1));
    }
}

void startServer(const net::ip::address &address, unsigned short port, const std::shared_ptr<std::string> &doc_root, int threads) {
    // The io_context is required for all I/O
    net::io_context ioc{threads};

    // Spawn a listening port
    boost::asio::spawn(ioc,
                       std::bind(
                               &do_listen,
                               std::ref(ioc),
                               tcp::endpoint{address, port},
                               doc_root,
                               std::placeholders::_1));

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for (auto i = threads - 1; i > 0; --i)
        v.emplace_back(
                [&ioc] {
                    ioc.run();
                });
    ioc.run();
}

void startServer(int argc, char *argv[]) {
    // Check command line arguments.
    if (argc != 5) {
        std::cerr <<
                  "Usage: <address> <port> <doc_root> <threads>\n" <<
                  "Example:\n" <<
                  "    ./webServer 0.0.0.0 8080 . 1\n";
        return;
    }
    auto const address = net::ip::make_address(argv[1]);
    auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
    auto const doc_root = std::make_shared<std::string>(argv[3]);
    auto const threads = std::max<int>(1, std::atoi(argv[4]));

    startServer(address, port, doc_root, threads);
}
