#define CPPHTTPLIB_THREAD_POOL_COUNT 8
#include "httplib.h"

#include "kv-store.h"
#include "db.h"
#include <iostream>

kv_store cache;
std::atomic<long long> cache_hits{0};
std::atomic<long long> cache_misses{0};

int main()
{
    db_init();
    init(&cache);

    httplib::Server server;

    server.Post("/create", [](const httplib::Request &req, httplib::Response &res)
                {
        auto key = req.get_param_value("key");
        auto value = req.get_param_value("value");

       if (db_insert(key.c_str(), value.c_str())) {
        // Then update cache
        bool created = kv_create(&cache, (char *)key.c_str(), 
                               (char *)value.c_str(), value.size());
        res.status = created ? 201 : 200;
        std::cout << "[CREATE] key=" << key << 
                    (created ? " inserted" : " updated") << " (cache+db)\n";
        res.set_content(created ? "Inserted\n" : "Updated\n", "text/plain");
    } else {
        res.set_content("DB Error\n", "text/plain");
        res.status = 500;
    } });

    server.Get("/get", [](const httplib::Request &req, httplib::Response &res)
               {
        auto key = req.get_param_value("key");
        char *val = kv_get(&cache, (char*)key.c_str());
        if (val) {
            // cache hit
            cache_hits.fetch_add(1);
            std::cout << "[CACHE HIT] key=" << key << " value=" << val << "\n";
            res.set_content(val, "text/plain");
           
            return;
        }

        // Cache miss -> go to DB
        cache_misses.fetch_add(1);
        std::cout << "[CACHE MISS] key=" << key << " -> querying DB\n";
        char *dbval = db_get(key.c_str());

        if (dbval) {
            std::cout << "[DB HIT] key=" << key << " value=" << dbval << "\n";
            // Insert into cache 
            kv_create(&cache, (char*)key.c_str(), dbval, strlen(dbval));
            res.set_content(dbval, "text/plain");
            free(dbval); 
        } else {
            std::cout << "[DB MISS] key=" << key << " not found\n";
            res.status = 404;
            res.set_content("Not found", "text/plain");
        } });

    server.Delete("/delete", [](const httplib::Request &req, httplib::Response &res)
                  {
        auto key = req.get_param_value("key");
        bool removed = kv_delete(&cache, (char*)key.c_str());
        if (!db_delete(key.c_str())) {
            res.status = 500;
            res.set_content("Database error\n", "text/plain");
            return;
        }
        res.status = removed ? 200 : 404;
        if (removed)
            std::cout << "[DELETE] key=" << key << " removed (cache+db)\n";
        else
            std::cout << "[DELETE] key=" << key << " not found in cache (still removed from db)\n";

        res.set_content("Deleted\n", "text/plain"); });

    server.Post("/preload", [](const httplib::Request &req, httplib::Response &res)
                {
    auto key = req.get_param_value("key");
    auto value = req.get_param_value("value");
    db_insert(key.c_str(), value.c_str());
    res.set_content("Preloaded", "text/plain"); });

    server.Delete("/clear", [](const httplib::Request &req, httplib::Response &res)
                  {
        db_clear();   
        init(&cache);
        cache_hits = 0;
        cache_misses = 0;
        std::cout << "[SERVER] Database and cache cleared.\n";
        res.set_content("Database and cache cleared.\n", "text/plain"); });

    server.Get("/cache-stats", [](const httplib::Request &req, httplib::Response &res)
               {
        long long total_accesses = cache_hits.load() + cache_misses.load();
        long long hits = cache_hits.load();
        long long misses = cache_misses.load();
        double hit_rate = total_accesses > 0 ? (double)hits/(double)total_accesses*100.0 : 0.0;

        std::string resp = "TotalAccesses:" + std::to_string(total_accesses) + 
                            "\nHits:" + std::to_string(hits) + 
                            "\nMisses:" + std::to_string(misses) + 
                            "\nHitRate:" + std::to_string(hit_rate) + "%\n";
        res.set_content(resp, "text/plain");
        res.status=200; });

    std::cout << "Server running on http://localhost:8080\n";
    server.listen("0.0.0.0", 8080);
    db_close();
    return 0;
}