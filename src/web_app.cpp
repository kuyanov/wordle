#include <iostream>
#include <sstream>
#include <uWebSockets/App.h>

#include "players.h"

const int port = 3000;
const int max_moves = 6;

struct UserData {
    std::unique_ptr<Host> host;
    int move = 0;
};

std::string ReadFile(const std::string &filename) {
    std::ifstream fin(filename);
    std::stringstream buffer;
    buffer << fin.rdbuf();
    return buffer.str();
}

int main() {
    auto dict = ReadDictionary("dictionary.txt");
    uWS::App().get("/", [&](auto *res, auto *req) {
        res->writeHeader("Content-Type", "text/html")->end(ReadFile("static/index.html"));
    }).get("/main.css", [&](auto *res, auto *req) {
        res->writeHeader("Content-Type", "text/css")->end(ReadFile("static/main.css"));
    }).get("/main.js", [&](auto *res, auto *req) {
        res->writeHeader("Content-Type", "application/javascript")->end(ReadFile("static/main.js"));
    }).get("/favicon.ico", [&](auto *res, auto *req) {
        res->writeHeader("Content-Type", "image/x-icon")->end(ReadFile("static/favicon.ico"));
    }).get("/*", [&](auto *res, auto *req) {
        res->writeStatus("404 Not Found")->end();
    }).ws<UserData>("/:mode", {
            .upgrade = [&](auto *res, auto *req, auto *context) {
                std::unique_ptr<Host> host;
                if (req->getParameter(0) == "random") {
                    host = std::make_unique<HostRandom>(dict);
                } else if (req->getParameter(0) == "hater") {
                    host = std::make_unique<HostHater>(dict);
                } else {
                    res->writeStatus("404 Not Found")->end();
                    return;
                }
                res->template upgrade<UserData>({.host = std::move(host)},
                                                req->getHeader("sec-websocket-key"),
                                                req->getHeader("sec-websocket-protocol"),
                                                req->getHeader("sec-websocket-extensions"),
                                                context);
            },
            .message = [&](auto *ws, std::string_view message, uWS::OpCode op_code) {
                if (message.empty()) {
                    return;
                }
                Host *host = ws->getUserData()->host.get();
                int &move = ws->getUserData()->move;
                if (std::find(dict.begin(), dict.end(), message) == dict.end()) {
                    ws->send("", op_code);
                } else if (move < max_moves) {
                    auto result = host->OnGuess(std::string(message));
                    ws->send(result, op_code);
                    if (++move == max_moves && std::count(result.begin(), result.end(), 'g') != result.size()) {
                        ws->send("!" + host->GetAnswer(), op_code);
                    }
                }
            },
            .close = [&](auto *ws, int code, std::string_view message) {
                ws->getUserData()->host.reset();
            }
    }).listen(port, [](auto *listen_socket) {
        if (listen_socket) {
            std::cout << "Listening on port " << port << std::endl;
        } else {
            std::cout << "Failed to listen on port " << port << std::endl;
        }
    }).run();
}
