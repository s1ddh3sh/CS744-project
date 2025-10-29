#include <libpq-fe.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

#include "db.h"

PGconn *conn;
static pthread_mutex_t db_mutex = PTHREAD_MUTEX_INITIALIZER;

void db_init()
{
    pthread_mutex_lock(&db_mutex);

    conn = PQconnectdb("host=localhost dbname=kvstore user=postgres password=12345");
    if (PQstatus(conn) != CONNECTION_OK)
    {
        fprintf(stderr, "Connection failed: %s", PQerrorMessage(conn));
        PQfinish(conn);
        exit(1);
    }
    PGresult *res = PQexec(conn, "CREATE TABLE IF NOT EXISTS kv_table (key TEXT PRIMARY KEY, value TEXT);");
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        fprintf(stderr, "[DB ERROR] Table creation failed: %s\n", PQerrorMessage(conn));
    }
    PQclear(res);

    pthread_mutex_unlock(&db_mutex);
}

bool db_insert(const char *key, const char *value)
{
    char query[512];
    snprintf(query, sizeof(query),
             "INSERT INTO kv_table VALUES ('%s','%s') "
             "ON CONFLICT (key) DO UPDATE SET value = EXCLUDED.value;",
             key, value);
    pthread_mutex_lock(&db_mutex);
    PGresult *res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        fprintf(stderr, "[DB ERROR] Insert failed for key='%s': %s\n", key, PQerrorMessage(conn));
        return 0;
    }
    PQclear(res);
    pthread_mutex_unlock(&db_mutex);
    return 1;
}

char *db_get(const char *key)
{
    char query[512];
    snprintf(query, sizeof(query), "SELECT value FROM kv_table WHERE key='%s';", key);
    pthread_mutex_lock(&db_mutex);
    PGresult *res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        fprintf(stderr, "[DB ERROR] Query failed for key='%s': %s\n",
                key, PQerrorMessage(conn));
        PQclear(res);
        pthread_mutex_unlock(&db_mutex);
        return NULL;
    }

    if (PQntuples(res) == 0)
    {
        PQclear(res);
        pthread_mutex_unlock(&db_mutex);
        return NULL;
    }

    char *val = strdup(PQgetvalue(res, 0, 0));
    PQclear(res);
    pthread_mutex_unlock(&db_mutex);
    return val;
}

bool db_delete(const char *key)
{
    char query[512];
    snprintf(query, sizeof(query), "DELETE FROM kv_table WHERE key='%s';", key);
    pthread_mutex_lock(&db_mutex);
    PGresult *res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        fprintf(stderr, "[DB ERROR] Delete failed for key='%s': %s\n",
                key, PQerrorMessage(conn));
        return 0;
    }
    PQclear(res);
    pthread_mutex_unlock(&db_mutex);
    return 1;
}
void db_clear()
{
    pthread_mutex_lock(&db_mutex);
    PQexec(conn, "DELETE FROM kv_table;");
    pthread_mutex_unlock(&db_mutex);
}

void db_close()
{
    pthread_mutex_lock(&db_mutex);
    if (conn)
    {
        PQfinish(conn);
        conn = NULL;
    }
    pthread_mutex_unlock(&db_mutex);
}