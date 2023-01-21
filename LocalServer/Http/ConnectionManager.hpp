#ifndef HTTP_CONNECTION_MANAGER_HPP
#define HTTP_CONNECTION_MANAGER_HPP

#include <set>
#include "Connection.hpp"


namespace HttpServer {

class ConnectionManager {
public:
    ConnectionManager(const ConnectionManager &) = delete;
    ConnectionManager &operator=(const ConnectionManager &) = delete;

    ConnectionManager();

    void start(const ConnectionPtr &c);
    void stop(const ConnectionPtr &c);

    void stopAll();

private:
    std::set<ConnectionPtr>            m_connections;

};

} // namespace HttpServer

#endif // HTTP_CONNECTION_MANAGER_HPP
