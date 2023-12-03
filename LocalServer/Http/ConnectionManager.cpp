#include "ConnectionManager.hpp"


namespace HttpServer {

ConnectionManager::ConnectionManager() {
}

void ConnectionManager::start(const ConnectionPtr &c) {
    m_connections.insert(c);
    c->start();
}

void ConnectionManager::stop(const ConnectionPtr &c) {
    m_connections.erase(c);
    c->stop();
}

void ConnectionManager::stopAll() {
    for (auto c : m_connections)
        c->stop();
    m_connections.clear();
}

} // namespace HttpServer
