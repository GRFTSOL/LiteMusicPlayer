//
//  Server.cpp
//  Mp3Player
//
//  Created by henry_xiao on 2023/1/21.
//

#include "Server.hpp"
#include "Utils/Utils.h"
#include "rapidjson/reader.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/document.h"
#include "rapidjson/error/error.h"
#include "rapidjson/error/en.h"


using namespace rapidjson;

namespace WebSocket {

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

Server::Server(int port) : m_port(port) {
}

void Server::on_message(Server *server, websocketpp::connection_hdl hdl, message_ptr msg) {
    rapidjson::Document doc;
    auto &payload = msg->get_payload();
    ParseResult ok = doc.Parse(payload.c_str(), payload.size());
    if (!ok) {
        DBG_LOG3("JSON parse error: %s (offset: %l), %s\n",
            rapidjson::GetParseError_En(ok.Code()),
            (unsigned int)ok.Offset(), payload.c_str() + ok.Offset());
        return;
    }

    assert(doc.IsObject());
    auto itType = doc.FindMember("type");
    if (itType == doc.MemberEnd()) {
        DBG_LOG1("No 'type' for message: %s", payload.c_str());
        return;
    }

    auto &type = (*itType).value;
    if (!type.IsString()) {
        DBG_LOG1("'type' is not String, for message: %s", payload.c_str());
        return;
    }
    auto it = server->m_mapHandlers.find(type.GetString());
    if (it == server->m_mapHandlers.end()) {
        DBG_LOG1("NO handler, for message: %s", payload.c_str());
        return;
    }

    DBG_LOG1("Handle Websocket message: %s", type.GetString());

    (*it).second->onMessage(hdl, doc);

//    std::cout << "on_message called with hdl: " << hdl.lock().get()
//              << " and message: " << msg->get_payload()
//              << "Opcode: " << msg->get_opcode()
//              << std::endl;
//
//    auto &ws = server->m_server;
//
//    try {
//        ws.send(hdl, msg->get_payload(), msg->get_opcode());
//    } catch (websocketpp::exception const & e) {
//        std::cout << "Echo failed because: "
//                  << "(" << e.what() << ")" << std::endl;
//    }
}

void Server::send(const websocketpp::connection_hdl &connection, const std::string &payload) {
    try {
        m_server.send(connection, payload, websocketpp::frame::opcode::text);
    } catch (websocketpp::exception const & e) {
        std::cout << "Echo failed because: "
                  << "(" << e.what() << ")" << std::endl;
    }
}

void Server::run() {
    // Set logging settings
    m_server.set_access_channels(websocketpp::log::alevel::all);
    m_server.clear_access_channels(websocketpp::log::alevel::frame_payload);

    m_server.init_asio();

    m_server.set_message_handler(bind(&on_message, this, _1, _2));

    m_server.listen(m_port);

    m_server.start_accept();

    m_server.run();
}

} // namespace WebSocket
