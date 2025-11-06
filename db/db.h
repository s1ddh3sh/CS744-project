#ifndef DB_H
#define DB_H

void db_init();
bool db_insert(const char *key, const char *value);
char *db_get(const char *key);
bool db_delete(const char *key);
void db_close();
void db_clear();

#endif
