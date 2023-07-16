#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <App.h>

#include "players.h"

const int port = 3000;
const int max_moves = 6;

struct GameData {
    std::unique_ptr<Host> host;
    int move = 0;
};

struct UserData {
    std::string id;
    GameData *game;
};

std::string ReadFile(const std::string &filename) {
    std::ifstream fin(filename);
    std::stringstream buffer;
    buffer << fin.rdbuf();
    return buffer.str();
}

int main() {
    Init();
    std::unordered_map<std::string, GameData> games;
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
    }).ws<UserData>("/:mode/:id", {
            .upgrade = [&](auto *res, auto *req, auto *context) {
                std::unique_ptr<Host> host;
                if (req->getParameter(0) == "random") {
                    host = std::make_unique<HostRandom>(true);
                } else if (req->getParameter(0) == "hater") {
                    host = std::make_unique<HostHater>(0.2, true);
                } else {
                    res->writeStatus("404 Not Found")->end();
                    return;
                }
                std::string id(req->getParameter(1));
                auto it = games.find(id);
                if (it == games.end()) {
                    it = games.insert({id, GameData{.host = std::move(host)}}).first;
                }
                res->template upgrade<UserData>({.id = id, .game = &it->second},
                                                req->getHeader("sec-websocket-key"),
                                                req->getHeader("sec-websocket-protocol"),
                                                req->getHeader("sec-websocket-extensions"),
                                                context);
            },
            .message = [&](auto *ws, std::string_view message, uWS::OpCode op_code) {
                auto &[host, move] = *ws->getUserData()->game;
                size_t guess_id = std::find(all.begin(), all.end(), message) - all.begin();
                if (guess_id == all.size()) {
                    ws->send("", op_code);
                } else if (move < max_moves) {
                    u_char pat = host->OnGuess(guess_id);
                    std::string pattern = DecodePattern(pat);
                    ws->send(pattern, op_code);
                    ++move;
                    if (move == max_moves && !IsAllGreen(pat)) {
                        ws->send("!" + all[host->GetAnswer()], op_code);
                    }
                    if (move == max_moves || IsAllGreen(pat)) {
                        games.erase(ws->getUserData()->id);
                    }
                }
            }
    }).listen(port, [](auto *listen_socket) {
        if (listen_socket) {
            std::cerr << "Listening on port " << port << std::endl;
        } else {
            std::cerr << "Failed to listen on port " << port << std::endl;
            exit(1);
        }
    }).run();
}
