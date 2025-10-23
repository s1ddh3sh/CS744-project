#ifndef DB_H
#define DB_H

void db_init();
void db_insert(const char *key, const char *value);
char *db_get(const char *key);
void db_delete(const char *key);
void db_close();

#endif
