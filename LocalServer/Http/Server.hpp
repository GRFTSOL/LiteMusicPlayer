﻿
#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include <asio.hpp>
#include <string>
#include "Connection.hpp"
#include "ConnectionManager.hpp"
#include "IRequestHandler.hpp"


namespace HttpServer {

class Server {
public:
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    explicit Server(const std::string &address, const std::string &port);

    void run();

    IRequestHandlerPtr getRequestHandler(const std::string &url);

    void registerRequestHandler(const IRequestHandlerPtr &handler);

    void stopConnection(const ConnectionPtr &connection);

private:
    void doAccept();

    void doAwaitStop();

    asio::io_context                m_ioContext;

    /// The signal_set is used to register for process termination notifications.
    asio::signal_set                m_signals;

    /// Acceptor used to listen for incoming connections.
    asio::ip::tcp::acceptor         m_acceptor;

    /// The connection manager which owns all live connections.
    ConnectionManager               m_connectionManager;

    VecRequestHandler               m_reqHandlers;

};

} // namespace HttpServer

#endif
