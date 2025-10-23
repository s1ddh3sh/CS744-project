#include <libpq-fe.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "db.h"

PGconn *conn;

void db_init()
{
    conn = PQconnectdb("host=localhost dbname=kvstore user=postgres password=12345");
    if (PQstatus(conn) != CONNECTION_OK)
    {
        fprintf(stderr, "Connection failed: %s", PQerrorMessage(conn));
        PQfinish(conn);
        exit(1);
    }
    PQexec(conn, "CREATE TABLE IF NOT EXISTS kv_table (key TEXT PRIMARY KEY, value TEXT);");
}

void db_insert(const char *key, const char *value)
{
    char query[512];
    snprintf(query, sizeof(query),
             "INSERT INTO kv_table VALUES ('%s','%s') "
             "ON CONFLICT (key) DO UPDATE SET value = EXCLUDED.value;",
             key, value);
    PQexec(conn, query);
}

char *db_get(const char *key)
{
    char query[512];
    snprintf(query, sizeof(query), "SELECT value FROM kv_table WHERE key='%s';", key);
    PGresult *res = PQexec(conn, query);
    if (PQntuples(res) > 0)
    {
        char *val = strdup(PQgetvalue(res, 0, 0));
        PQclear(res);
        return val;
    }
    PQclear(res);
    return NULL;
}

void db_delete(const char *key)
{
    char query[512];
    snprintf(query, sizeof(query), "DELETE FROM kv_table WHERE key='%s';", key);
    PQexec(conn, query);
}

void db_close()
{
    PQfinish(conn);
}