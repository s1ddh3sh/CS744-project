#include "httplib.h"

#include "kv-store.h"
#include "db.h"
#include <iostream>

kv_store cache;

int main()
{
    db_init();
    init(&cache);

    httplib::Server server;

    server.Post("/create", [](const httplib::Request &req, httplib::Response &res)
                {
        auto key = req.get_param_value("key");
        auto value = req.get_param_value("value");
        
        kv_create(&cache, (char*)key.c_str(),(char*)value.c_str(), value.size()); 
        //insert into db
        db_insert(key.c_str(), value.c_str());
        std::cout << "[CREATE] key=" << key << " inserted.\n";

        res.set_content("Inserted", "text/plain"); });

    server.Get("/get", [](const httplib::Request &req, httplib::Response &res)
               {
        auto key = req.get_param_value("key");
        char *val = kv_get(&cache, (char*)key.c_str());
        if(val == NULL){
            // cache miss, get from db
            std::cout << "[CACHE MISS] key=" << key << " -> fetching from DB\n";

            val = db_get(key.c_str());
            if(val) {
                kv_create(&cache, (char*)key.c_str(), val, strlen(val));
                std::cout << "[DB HIT] key=" << key << " value=" << val << "\n";
            }
            else {
                std::cout << "[DB MISS] key=" << key << "\n";
            }
        }
        else {
            std::cout << "[CACHE HIT] key=" << key << " value=" << val << "\n";
        }
        res.set_content(val ? val : "Not found", "text/plain"); });

    server.Delete("/delete", [](const httplib::Request &req, httplib::Response &res)
                  {
        auto key = req.get_param_value("key");
        kv_delete(&cache, (char*)key.c_str());
        db_delete(key.c_str());
        std::cout << "[DELETE] key=" << key << " deleted\n";
        res.set_content("Deleted\n", "text/plain"); });

    std::cout << "Server running on http://localhost:8080\n";
    server.listen("0.0.0.0", 8080);
    db_close();
}