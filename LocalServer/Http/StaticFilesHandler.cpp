#include "StaticFilesHandler.hpp"
#include "Connection.hpp"
#include "Utils/Utils.h"
#include "Utils/url.h"


namespace HttpServer {

const std::string &guestMimeType(const string &fn) {
    struct ExtToMime {
        const std::string extension;
        const std::string mime_type;
    };

    static ExtToMime mappings[] = {
        { "htm", "text/html" },
        { "html", "text/html" },
        { "css", "text/css" },
        { "js", "application/x-javascript;" },
        { "jpg", "image/jpeg" },
        { "png", "image/png" },
        { "gif", "image/gif" },
    };

    static const std::string defaultMime = "text/plain";

    cstr_t ext = urlGetExt(fn.c_str());
    if (*ext == '.') {
        ext++;
    }

    for (auto &m : mappings) {
        if (m.extension == ext) {
            return m.mime_type;
        }
    }

    return defaultMime;
}

void sendFile(const ConnectionPtr &connection, const string &fn) {
    auto &response = connection->response();

    response.addHeader(HEADER_CONTENT_TYPE, guestMimeType(fn));

    if (!readFile(fn.c_str(), response.body)) {
        response.status = Response::INTERNAL_SERVER_ERROR;
    }

    response.status = Response::OK;
    connection->sendResponse();
}

bool isValidRequestUri(const string &path) {
    const char *illegalNames[] = {
        "../",
        "..\\",
    };

    for (int i = 0; i < CountOf(illegalNames); ++i) {
        if (path.find(illegalNames[i]) != -1) {
            return false;
        }
    }

    return true;
}

string searchDefaultHtmlFile(const string &path) {
    const char *names[] = {
        "default.html",
        "index.html",
    };

    for (int i = 0; i < CountOf(names); ++i) {
        string fn = dirStringJoin(path.c_str(), names[i]);
        if (isFileExist(fn.c_str())) {
            return fn;
        }
    }

    return "";
}

StaticFilesHandler::StaticFilesHandler(const string &uriPath, const string &pathOrFile) : m_uriPath(uriPath), m_pathOrFile(pathOrFile)
{
    if (!isFileExist(m_pathOrFile.c_str())) {
        ERR_LOG1("Static file/directory does NOT exist: %s", pathOrFile.c_str());
    }
}

bool StaticFilesHandler::onRequestHeader(const ConnectionPtr &connection) {
    auto &req = connection->request();

    auto uri = req.uri.substr(m_uriPath.size());
    uri = uriUnquote(uri.c_str());
    if (!isValidRequestUri(uri)) {
        connection->sendStockResponse(Response::BAD_REQUEST);
        return true;
    }

    string fn;
    if (isDirExist(m_pathOrFile.c_str())) {
        // Serve directory.
        fn = dirStringJoin(m_pathOrFile.c_str(), uri.c_str());
        if (isDirExist(fn.c_str())) {
            fn = searchDefaultHtmlFile(fn);
        } else if (!isFileExist(fn.c_str())) {
            fn.clear();
        }
    } else {
        // Serve file only
        if (uri == m_uriPath) {
            fn = m_pathOrFile;
        }
    }

    if (!fn.empty() && isFileExist(fn.c_str())) {
        sendFile(connection, fn);
        return true;
    }

    connection->sendStockResponse(Response::NOT_FOUND);
    return true;
}

bool StaticFilesHandler::onRequestBody(const ConnectionPtr &connection) {
    auto &response = connection->response();

    connection->sendStockResponse(Response::NOT_FOUND);
    return true;
}

} // namespace HttpServer
