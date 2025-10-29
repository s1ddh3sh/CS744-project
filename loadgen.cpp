// Usage: ./loadgen 10 60 get_popular
#include "httplib.h"
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <iostream>
#include <random>
#include <string>
#include <cstring>

using namespace std::chrono;

// config
const std::string SERVER_HOST = "localhost";
const int SERVER_PORT = 8080;
const int KEYSPACE_SIZE = 10000; // for get_all and mixed
const int POPULAR_SET_SIZE = 10; // for get_popular
const int MIXED_PUT_PCT = 20;    // 20% PUT
const int MIXED_DELETE_PCT = 10; // 10% DELETE, 70% GET

enum WorkloadType
{
    PUT_ALL,
    GET_ALL,
    GET_POPULAR,
    MIXED
};

struct GlobalStats
{
    std::atomic<long long> successful_requests{0};
    std::atomic<long long> failed_requests{0};
    std::atomic<long long> total_latency_ns{0};
};

struct ThreadArgs
{
    int thread_id;
    int duration_sec;
    WorkloadType workload;
    GlobalStats *gstats;
};

// HTTP
static void do_post_create(httplib::Client &cli, const std::string &key, const std::string &value,
                           bool &success, long long &latency_ns)
{
    auto start = high_resolution_clock::now();
    auto res = cli.Post("/create", ("key=" + key + "&value=" + value),
                        "application/x-www-form-urlencoded");
    auto end = high_resolution_clock::now();
    latency_ns = duration_cast<nanoseconds>(end - start).count();
    success = (res && res->status >= 200 && res->status < 300);
}

static void do_get(httplib::Client &cli, const std::string &key,
                   bool &success, long long &latency_ns)
{
    auto start = high_resolution_clock::now();
    auto res = cli.Get(("/get?key=" + key).c_str());
    auto end = high_resolution_clock::now();
    latency_ns = duration_cast<nanoseconds>(end - start).count();
    success = (res && res->status >= 200 && res->status < 300);
}

static void do_delete(httplib::Client &cli, const std::string &key,
                      bool &success, long long &latency_ns)
{
    auto start = high_resolution_clock::now();
    auto res = cli.Delete(("/delete?key=" + key).c_str());
    auto end = high_resolution_clock::now();
    latency_ns = duration_cast<nanoseconds>(end - start).count();
    success = (res && res->status >= 200 && res->status < 300);
}

// Worker thread
void client_thread(ThreadArgs args)
{
    std::mt19937 rng(std::random_device{}());
    httplib::Client cli(SERVER_HOST.c_str(), SERVER_PORT);
    cli.set_connection_timeout(3);
    cli.set_read_timeout(5);
    cli.set_write_timeout(3);

    long long seq = 0;
    auto end_time = high_resolution_clock::now() + seconds(args.duration_sec);
    std::uniform_int_distribution<int> mix_dist(1, 100);

    // Prepopulate small popular set for get_popular
    std::vector<std::string> popular_keys;
    if (args.workload == GET_POPULAR)
    {
        for (int i = 0; i < POPULAR_SET_SIZE; ++i)
        {
            std::string k = "popular_" + std::to_string(i);
            std::string v = "val_" + std::to_string(i);
            bool s = false;
            long long lat = 0;
            do_post_create(cli, k, v, s, lat);
            popular_keys.push_back(k);
        }
    }

    while (high_resolution_clock::now() < end_time)
    {
        bool success = false;
        long long latency_ns = 0;

        switch (args.workload)
        {
        case PUT_ALL:
        {
            std::string key = "put_" + std::to_string(args.thread_id) + "_" + std::to_string(seq++);
            std::string val = "v" + std::to_string(seq);
            do_post_create(cli, key, val, success, latency_ns);
            break;
        }
        case GET_ALL:
        {
            std::string key = "get_" + std::to_string(seq++ % KEYSPACE_SIZE);
            do_get(cli, key, success, latency_ns);
            break;
        }
        case GET_POPULAR:
        {
            std::uniform_int_distribution<int> pick(0, POPULAR_SET_SIZE - 1);
            std::string key = popular_keys[pick(rng)];
            do_get(cli, key, success, latency_ns);
            break;
        }
        case MIXED:
        {
            int r = mix_dist(rng);
            if (r <= MIXED_PUT_PCT)
            {
                std::string key = "mput_" + std::to_string(args.thread_id) + "_" + std::to_string(seq++);
                std::string val = "mv" + std::to_string(seq);
                do_post_create(cli, key, val, success, latency_ns);
            }
            else if (r <= MIXED_PUT_PCT + MIXED_DELETE_PCT)
            {
                std::string key = "mput_" + std::to_string(args.thread_id) + "_" + std::to_string(std::max(0LL, seq - 10));
                do_delete(cli, key, success, latency_ns);
            }
            else
            {
                std::string key = "get_" + std::to_string(seq++ % KEYSPACE_SIZE);
                do_get(cli, key, success, latency_ns);
            }
            break;
        }
        }

        if (success)
        {
            args.gstats->successful_requests.fetch_add(1);
            args.gstats->total_latency_ns.fetch_add(latency_ns);
        }
        else
        {
            args.gstats->failed_requests.fetch_add(1);
        }
    }
}

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        std::cerr << "Usage: " << argv[0] << " <threads> <duration_sec> <workload>\n"
                  << "Workloads: put_all | get_all | get_popular | mixed\n";
        return 1;
    }

    int threads = atoi(argv[1]);
    int duration = atoi(argv[2]);
    std::string workload_str = argv[3];
    WorkloadType workload;

    if (workload_str == "put_all")
        workload = PUT_ALL;
    else if (workload_str == "get_all")
        workload = GET_ALL;
    else if (workload_str == "get_popular")
        workload = GET_POPULAR;
    else if (workload_str == "mixed")
        workload = MIXED;
    else
    {
        std::cerr << "Invalid workload type.\n";
        return 1;
    }

    std::cout << "Loadgen starting (" << threads << " threads, "
              << duration << "s, workload=" << workload_str << ")\n";

    GlobalStats gstats;
    std::vector<std::thread> pool;
    pool.reserve(threads);

    auto start = high_resolution_clock::now();
    for (int i = 0; i < threads; ++i)
    {
        ThreadArgs args{i, duration, workload, &gstats};
        pool.emplace_back(client_thread, args);
    }

    for (auto &t : pool)
        t.join();
    auto end = high_resolution_clock::now();

    std::chrono::duration<double> elapsed_d = end - start;
    double elapsed = elapsed_d.count();
    long long succ = gstats.successful_requests.load();
    long long fail = gstats.failed_requests.load();
    long long total_ns = gstats.total_latency_ns.load();

    double throughput = succ / elapsed;
    double avg_latency_ms = (succ > 0) ? (total_ns / 1e6) / succ : 0.0;

    std::cout << "\n===== Results =====\n"
              << "Successful requests: " << succ << "\n"
              << "Failed requests: " << fail << "\n"
              << "Average throughput (req/s): " << throughput << "\n"
              << "Average response time (ms): " << avg_latency_ms << "\n"
              << "=====================\n";
}
