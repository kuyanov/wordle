#include <iostream>
#include <sstream>
#include <uWebSockets/App.h>

#include "game.h"

const int port = 3000;

std::string ReadFile(const std::string &filename) {
    std::ifstream fin(filename);
    std::stringstream buffer;
    buffer << fin.rdbuf();
    return buffer.str();
}

int main() {
    auto dict = ReadDict("dictionary.txt");
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
    }).listen(port, [](auto *listen_socket) {
        if (listen_socket) {
            std::cout << "Listening on port " << port << std::endl;
        } else {
            std::cout << "Failed to listen on port " << port << std::endl;
        }
    }).run();
}
