// Copyright 2020 Kavykin Andrey Kaviandr@yandex.ru

#pragma once

#include <atomic>
#include <condition_variable>
#include <fstream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <web_crawler_lib/parser.hpp>
#include <web_crawler_lib/url_reader.hpp>

namespace web_crawler_lib {

    class WebCrawler final {
    public:

        void join();

        static WebCrawler from(::std::string const& url, ::std::size_t depth, ::std::size_t network_threads,
                               ::std::size_t parser_threads, ::std::string const& output_file_name);

    private:

        const ::std::size_t max_depth_;


        ::std::ofstream output_;


        asio::thread_pool network_workers_;
        asio::thread_pool parser_workers_;
        ::std::thread writer_worker_;

        ::std::atomic_size_t active_jobs_ = 0;
        ::std::mutex active_jobs_mutex_{};
        ::std::condition_variable active_jobs_cv_{};

        struct NetworkJob {
            ::std::string url;
            ::std::size_t depth;
        };

        struct ParserJob {
            http_response_t response;
            ::std::size_t depth;
        };

        ::std::queue<::std::string> writer_queue_{};
        ::std::mutex writer_queue_mutex_{};
        ::std::condition_variable writer_queue_cv_{};


        WebCrawler(::std::size_t max_depth, ::std::string const& root_url, ::std::size_t network_workers,
                   ::std::size_t parser_workers, ::std::string const& output_file_name);

        void notify_start_job_();

        void notify_finish_job_();

        void publish_network_job(NetworkJob&& job);

        void publish_parser_job(ParserJob&& job);

        void publish_writer_job(::std::string&& job);
    };
}
