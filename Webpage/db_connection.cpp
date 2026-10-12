#include "db_connection.h"
#include <sstream>

DBConnection::DBConnection() : conn(nullptr), connected(false) {}

DBConnection::~DBConnection() {
#if HAS_LIBPQ
    if (conn) {
        PQfinish(conn);
        conn = nullptr;
    }
#endif
}

bool DBConnection::connect(const string& host, int port, const string& dbname, const string& user, const string& password) {
#if HAS_LIBPQ
    ostringstream ss;
    ss << "host=" << host 
       << " port=" << port 
       << " dbname=" << dbname 
       << " user=" << user 
       << " password=" << password
       << " connect_timeout=3";

    connString = ss.str();
    conn = PQconnectdb(connString.c_str());

    if (PQstatus(conn) != CONNECTION_OK) {
        cerr << "[PostgreSQL Error] Connection failed: " << PQerrorMessage(conn) << endl;
        PQfinish(conn);
        conn = nullptr;
        connected = false;
        return false;
    }

    connected = true;
    cout << "[PostgreSQL] Connected successfully to database: '" << dbname << "' on " << host << ":" << port << endl;
    createTable();
    return true;
#else
    cout << "[DB Module] libpq-fe.h not installed yet. (Run 'sudo apt install libpq-dev' to enable live PostgreSQL connection)" << endl;
    connected = false;
    return false;
#endif
}

bool DBConnection::isConnected() const {
#if HAS_LIBPQ
    return connected && conn && (PQstatus(conn) == CONNECTION_OK);
#else
    return false;
#endif
}

void DBConnection::createTable() {
#if HAS_LIBPQ
    if (!isConnected()) return;

    const char* sql = "CREATE TABLE IF NOT EXISTS users ("
                      "id SERIAL PRIMARY KEY, "
                      "name VARCHAR(100) NOT NULL, "
                      "email VARCHAR(100) NOT NULL, "
                      "mobile VARCHAR(20) NOT NULL);";

    PGresult* res = PQexec(conn, sql);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        cerr << "[PostgreSQL Error] Failed to create table: " << PQerrorMessage(conn) << endl;
    } else {
        cout << "[PostgreSQL] Checked/Created 'users' table successfully." << endl;
    }
    PQclear(res);
#endif
}

string DBConnection::escapeJSON(const string& s) {
    string res = "";
    for (char c : s) {
        if (c == '"') res += "\\\"";
        else if (c == '\\') res += "\\\\";
        else if (c == '\n') res += "\\n";
        else if (c == '\r') res += "\\r";
        else if (c == '\t') res += "\\t";
        else res += c;
    }
    return res;
}

// READ: SELECT all users as JSON
string DBConnection::getAllUsersJSON() {
#if HAS_LIBPQ
    if (!isConnected()) return "[]";

    const char* sql = "SELECT id, name, email, mobile FROM users ORDER BY id ASC;";
    PGresult* res = PQexec(conn, sql);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        cerr << "[PostgreSQL Error] Query failed: " << PQerrorMessage(conn) << endl;
        PQclear(res);
        return "[]";
    }

    int rows = PQntuples(res);
    ostringstream jsonStream;
    jsonStream << "[";

    for (int i = 0; i < rows; ++i) {
        string id = PQgetvalue(res, i, 0);
        string name = PQgetvalue(res, i, 1);
        string email = PQgetvalue(res, i, 2);
        string mobile = PQgetvalue(res, i, 3);

        jsonStream << "{"
                   << "\"id\":" << id << ","
                   << "\"name\":\"" << escapeJSON(name) << "\","
                   << "\"email\":\"" << escapeJSON(email) << "\","
                   << "\"mobile\":\"" << escapeJSON(mobile) << "\""
                   << "}";

        if (i + 1 < rows) jsonStream << ",";
    }

    jsonStream << "]";
    PQclear(res);
    return jsonStream.str();
#else
    return "[]";
#endif
}

// CREATE: INSERT new user record
string DBConnection::insertUser(const string& name, const string& email, const string& mobile) {
#if HAS_LIBPQ
    if (!isConnected()) return "{}";

    const char* sql = "INSERT INTO users (name, email, mobile) VALUES ($1, $2, $3) RETURNING id;";
    const char* paramValues[3] = { name.c_str(), email.c_str(), mobile.c_str() };

    PGresult* res = PQexecParams(conn, sql, 3, NULL, paramValues, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        cerr << "[PostgreSQL Error] Insert failed: " << PQerrorMessage(conn) << endl;
        PQclear(res);
        return "{}";
    }

    string newIdStr = PQgetvalue(res, 0, 0);
    PQclear(res);

    cout << "[PostgreSQL] Inserted user into DB: ID=" << newIdStr << ", Name=" << name << endl;

    ostringstream jsonStream;
    jsonStream << "{"
               << "\"id\":" << newIdStr << ","
               << "\"name\":\"" << escapeJSON(name) << "\","
               << "\"email\":\"" << escapeJSON(email) << "\","
               << "\"mobile\":\"" << escapeJSON(mobile) << "\""
               << "}";

    return jsonStream.str();
#else
    return "{}";
#endif
}

// UPDATE: UPDATE user record by ID
bool DBConnection::updateUser(int id, const string& name, const string& email, const string& mobile) {
#if HAS_LIBPQ
    if (!isConnected()) return false;

    string idStr = to_string(id);
    const char* sql = "UPDATE users SET name = $1, email = $2, mobile = $3 WHERE id = $4;";
    const char* paramValues[4] = { name.c_str(), email.c_str(), mobile.c_str(), idStr.c_str() };

    PGresult* res = PQexecParams(conn, sql, 4, NULL, paramValues, NULL, NULL, 0);

    bool success = (PQresultStatus(res) == PGRES_COMMAND_OK);
    if (!success) {
        cerr << "[PostgreSQL Error] Update failed: " << PQerrorMessage(conn) << endl;
    } else {
        cout << "[PostgreSQL] Updated user in DB: ID=" << id << endl;
    }

    PQclear(res);
    return success;
#else
    return false;
#endif
}

// DELETE: DELETE user record by ID
bool DBConnection::deleteUser(int id) {
#if HAS_LIBPQ
    if (!isConnected()) return false;

    string idStr = to_string(id);
    const char* sql = "DELETE FROM users WHERE id = $1;";
    const char* paramValues[1] = { idStr.c_str() };

    PGresult* res = PQexecParams(conn, sql, 1, NULL, paramValues, NULL, NULL, 0);

    bool success = (PQresultStatus(res) == PGRES_COMMAND_OK);
    if (!success) {
        cerr << "[PostgreSQL Error] Delete failed: " << PQerrorMessage(conn) << endl;
    } else {
        cout << "[PostgreSQL] Deleted user from DB: ID=" << id << endl;
    }

    PQclear(res);
    return success;
#else
    return false;
#endif
}
