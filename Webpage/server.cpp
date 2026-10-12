#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "db_connection.h"

using namespace std;

// Global PostgreSQL Database Manager Connection Instance
DBConnection g_db;

// In-Memory Fallback User Storage (Used if PostgreSQL database is offline)
struct UserFallback {
    int id;
    string name;
    string email;
    string mobile;

    string toJSON() const {
        return "{\"id\":" + to_string(id) + 
               ",\"name\":\"" + name + "\"" +
               ",\"email\":\"" + email + "\"" +
               ",\"mobile\":\"" + mobile + "\"}";
    }
};

vector<UserFallback> g_fallbackUsers;
int g_fallbackNextId = 101;

void initFallbackData() {
    g_fallbackUsers.push_back({g_fallbackNextId++, "John Doe", "john.doe@example.com", "+1-1234567890"});
    g_fallbackUsers.push_back({g_fallbackNextId++, "Alice Smith", "alice.smith@example.com", "+1-9876543210"});
}

// JSON Helper Extractor
string extractJSONValue(const string& json, const string& key) {
    string searchKey = "\"" + key + "\":";
    size_t pos = json.find(searchKey);
    if (pos == string::npos) return "";

    pos += searchKey.length();
    while (pos < json.length() && (json[pos] == ' ' || json[pos] == '"')) {
        pos++;
    }

    size_t endPos = pos;
    if (json[pos - 1] == '"') {
        while (endPos < json.length() && json[endPos] != '"') {
            if (json[endPos] == '\\') endPos++; // skip escaped char
            endPos++;
        }
    } else {
        while (endPos < json.length() && json[endPos] != ',' && json[endPos] != '}') {
            endPos++;
        }
    }
    return json.substr(pos, endPos - pos);
}

// ============================================================================
// C++ HTTP ROUTER & REQUEST HANDLER
// ============================================================================
void handleClient(int clientSocket) {
    char buffer[4096] = {0};
    ssize_t bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
    if (bytesRead <= 0) {
        close(clientSocket);
        return;
    }

    string request(buffer, bytesRead);
    istringstream requestStream(request);
    string method, path, protocol;
    requestStream >> method >> path >> protocol;

    // Extract Headers & Body
    size_t bodyPos = request.find("\r\n\r\n");
    string body = "";
    if (bodyPos != string::npos) {
        body = request.substr(bodyPos + 4);
    }

    ostringstream response;

    // Route 1: GET / -> Serve HTML Webpage UI
    if (method == "GET" && (path == "/" || path == "/index.html")) {
        ifstream file("index.html");
        if (file.is_open()) {
            stringstream fileBuffer;
            fileBuffer << file.rdbuf();
            string content = fileBuffer.str();
            
            response << "HTTP/1.1 200 OK\r\n"
                     << "Content-Type: text/html\r\n"
                     << "Content-Length: " << content.length() << "\r\n"
                     << "Access-Control-Allow-Origin: *\r\n\r\n"
                     << content;
        } else {
            response << "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
        }
    }
    // Route 2: GET /api/users -> Return Users from PostgreSQL Database
    else if (method == "GET" && path == "/api/users") {
        string jsonOutput;
        if (g_db.isConnected()) {
            jsonOutput = g_db.getAllUsersJSON();
        } else {
            // Fallback memory
            ostringstream jsonStream;
            jsonStream << "[";
            for (size_t i = 0; i < g_fallbackUsers.size(); ++i) {
                jsonStream << g_fallbackUsers[i].toJSON();
                if (i + 1 < g_fallbackUsers.size()) jsonStream << ",";
            }
            jsonStream << "]";
            jsonOutput = jsonStream.str();
        }

        response << "HTTP/1.1 200 OK\r\n"
                 << "Content-Type: application/json\r\n"
                 << "Content-Length: " << jsonOutput.length() << "\r\n"
                 << "Access-Control-Allow-Origin: *\r\n\r\n"
                 << jsonOutput;
    }
    // Route 3: POST /api/users -> Insert User into PostgreSQL Database
    else if (method == "POST" && path == "/api/users") {
        string name = extractJSONValue(body, "name");
        string email = extractJSONValue(body, "email");
        string mobile = extractJSONValue(body, "mobile");

        string jsonOutput;
        if (g_db.isConnected()) {
            jsonOutput = g_db.insertUser(name, email, mobile);
        } else {
            UserFallback u = {g_fallbackNextId++, name, email, mobile};
            g_fallbackUsers.push_back(u);
            jsonOutput = u.toJSON();
        }

        response << "HTTP/1.1 201 Created\r\n"
                 << "Content-Type: application/json\r\n"
                 << "Content-Length: " << jsonOutput.length() << "\r\n"
                 << "Access-Control-Allow-Origin: *\r\n\r\n"
                 << jsonOutput;
    }
    // Route 4: PUT /api/users -> Update User in PostgreSQL Database
    else if (method == "PUT" && path == "/api/users") {
        string idStr = extractJSONValue(body, "id");
        string name = extractJSONValue(body, "name");
        string email = extractJSONValue(body, "email");
        string mobile = extractJSONValue(body, "mobile");
        int targetId = stoi(idStr);

        bool updated = false;
        string jsonOutput = "{}";

        if (g_db.isConnected()) {
            updated = g_db.updateUser(targetId, name, email, mobile);
            if (updated) {
                jsonOutput = "{\"id\":" + idStr + ",\"name\":\"" + name + "\",\"email\":\"" + email + "\",\"mobile\":\"" + mobile + "\"}";
            }
        } else {
            for (auto& u : g_fallbackUsers) {
                if (u.id == targetId) {
                    u.name = name;
                    u.email = email;
                    u.mobile = mobile;
                    jsonOutput = u.toJSON();
                    updated = true;
                    break;
                }
            }
        }

        if (updated) {
            response << "HTTP/1.1 200 OK\r\n"
                     << "Content-Type: application/json\r\n"
                     << "Content-Length: " << jsonOutput.length() << "\r\n"
                     << "Access-Control-Allow-Origin: *\r\n\r\n"
                     << jsonOutput;
        } else {
            response << "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
        }
    }
    // Route 5: DELETE /api/users?id=X -> Delete User from PostgreSQL Database
    else if (method == "DELETE" && path.find("/api/users") == 0) {
        size_t queryPos = path.find("id=");
        if (queryPos != string::npos) {
            int targetId = stoi(path.substr(queryPos + 3));
            if (g_db.isConnected()) {
                g_db.deleteUser(targetId);
            } else {
                auto it = remove_if(g_fallbackUsers.begin(), g_fallbackUsers.end(), [targetId](const UserFallback& u) {
                    return u.id == targetId;
                });
                if (it != g_fallbackUsers.end()) {
                    g_fallbackUsers.erase(it, g_fallbackUsers.end());
                }
            }
        }

        string jsonOutput = "{\"success\":true}";
        response << "HTTP/1.1 200 OK\r\n"
                 << "Content-Type: application/json\r\n"
                 << "Content-Length: " << jsonOutput.length() << "\r\n"
                 << "Access-Control-Allow-Origin: *\r\n\r\n"
                 << jsonOutput;
    }
    // Route 6: CORS Preflight (OPTIONS)
    else if (method == "OPTIONS") {
        response << "HTTP/1.1 204 No Content\r\n"
                 << "Access-Control-Allow-Origin: *\r\n"
                 << "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n"
                 << "Access-Control-Allow-Headers: Content-Type\r\n\r\n";
    }
    else {
        response << "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
    }

    string resStr = response.str();
    send(clientSocket, resStr.c_str(), resStr.length(), 0);
    close(clientSocket);
}

int main() {
    initFallbackData();

    // Attempt connecting to PostgreSQL database
    cout << "[C++ Server] Connecting to PostgreSQL Database..." << endl;
    bool dbConnected = g_db.connect("localhost", 5432, "userdb", "postgres", "postgres");
    if (!dbConnected) {
        cout << "[C++ Server Note] PostgreSQL offline or libpq not configured. Operating in In-Memory Mode." << endl;
    }

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        cerr << "Error creating socket!" << endl;
        return 1;
    }

    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#ifdef SO_REUSEPORT
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
#endif

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "Error binding to port 8080! Run: fuser -k 8080/tcp" << endl;
        return 1;
    }

    if (listen(serverSocket, 10) < 0) {
        cerr << "Error listening on socket!" << endl;
        return 1;
    }

    cout << "=======================================================" << endl;
    cout << "   C++ BACKEND SERVER RUNNING ON http://localhost:8080  " << endl;
    cout << "   Database Module: Separate DBConnection Class        " << endl;
    cout << "   Database Mode: " << (dbConnected ? "PostgreSQL Live" : "In-Memory Mode") << endl;
    cout << "=======================================================" << endl;

    while (true) {
        sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
        if (clientSocket >= 0) {
            handleClient(clientSocket);
        }
    }

    close(serverSocket);
    return 0;
}
