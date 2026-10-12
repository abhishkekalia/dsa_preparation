#ifndef DB_CONNECTION_H
#define DB_CONNECTION_H

#include <string>
#include <vector>
#include <iostream>

#if __has_include(<libpq-fe.h>)
    #include <libpq-fe.h>
    #define HAS_LIBPQ 1
#else
    #define HAS_LIBPQ 0
    typedef void PGconn;
#endif

using namespace std;

class DBConnection {
private:
    PGconn* conn;
    string connString;
    bool connected;

public:
    DBConnection();
    ~DBConnection();

    // Connect to PostgreSQL database using connection parameters
    bool connect(const string& host = "localhost", 
                 int port = 5432, 
                 const string& dbname = "userdb", 
                 const string& user = "postgres", 
                 const string& password = "postgres");

    // Check if connected
    bool isConnected() const;

    // Table initialization
    void createTable();

    // CRUD operations interfacing with PostgreSQL
    string getAllUsersJSON();
    string insertUser(const string& name, const string& email, const string& mobile);
    bool updateUser(int id, const string& name, const string& email, const string& mobile);
    bool deleteUser(int id);

private:
    static string escapeJSON(const string& s);
};

#endif // DB_CONNECTION_H
