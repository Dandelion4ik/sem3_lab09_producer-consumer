// Copyright 2020 Kavykin Andrey Kaviandr@yandex.ru
#include <web_crawler_lib/web_crawler.hpp>

namespace web_crawler_lib {

WebCrawler::WebCrawler(::std::size_t const max_depth,
                       ::std::string const& root_url,
                       ::std::size_t const network_workers,
                       ::std::size_t const parser_workers,
                       ::std::string const& output_file_name)
    : max_depth_{max_depth},
      output_{output_file_name},
      network_workers_{network_workers},
      parser_workers_{parser_workers},
      writer_worker_{} {
  publish_network_job({root_url, 0});
  writer_worker_ = ::std::thread{[this] {
    while (true /* break used inside */) {
      ::std::unique_lock lock{writer_queue_mutex_};
      bool has_more_jobs;
      while (writer_queue_.empty() && (has_more_jobs = (active_jobs_ != 0))) {
        writer_queue_cv_.wait(lock);
      }
      if (!has_more_jobs)
        break;
      auto const result = writer_queue_.front();
      writer_queue_.pop();
      lock.unlock();
      output_ << result << ::std::endl;

      notify_finish_job_();
    }
  }};
}

void WebCrawler::join() {
  {
    ::std::unique_lock lock{active_jobs_mutex_};
    active_jobs_cv_.wait(lock, [this] { return active_jobs_ == 0; });
  }

  writer_worker_.join();
  network_workers_.join();
  parser_workers_.join();
}
WebCrawler WebCrawler::from(std::string const& url, ::std::size_t depth,
                            ::std::size_t network_threads,
                            ::std::size_t parser_threads,
                            std::string const& output_file_name) {
  return WebCrawler{depth, url,
                    network_threads == 0 ? ::std::thread::hardware_concurrency()
                                         : network_threads,
                    parser_threads == 0 ? ::std::thread::hardware_concurrency()
                                        : parser_threads,
                    output_file_name};
}

void WebCrawler::publish_network_job(NetworkJob&& job) {
  notify_start_job_();

  asio::post(network_workers_, [this, job] {
    auto const url_parts = parse_url_parts(job.url);
    if (url_parts) {
      if (auto response = read_from_url(
              url_parts->first, "80" /* default port */, url_parts->second))
        publish_parser_job({::std::move(*response), job.depth});
    }

    notify_finish_job_();
  });
}

void WebCrawler::publish_parser_job(ParserJob&& job) {
  notify_start_job_();

  asio::post(parser_workers_, [this, job] {
    auto const children = job.depth < max_depth_;
    auto result = parse_http_response(job.response, children);
    for (auto image_url : result.image_urls)
      publish_writer_job(::std::move(image_url));
    if (children) {
      auto const next_depth = job.depth + 1;
      for (auto child_url : result.child_urls)
        publish_network_job({::std::move(child_url), next_depth});
    }

    notify_finish_job_();
  });
}

void WebCrawler::publish_writer_job(::std::string&& job) {
  notify_start_job_();

  ::std::lock_guard lock{writer_queue_mutex_};
  writer_queue_.push(::std::move(job));
  writer_queue_cv_.notify_one();
}

void WebCrawler::notify_start_job_() {
  ::std::lock_guard lock{active_jobs_mutex_};
  ++active_jobs_;
  active_jobs_cv_.notify_all();
}

void WebCrawler::notify_finish_job_() {
  ::std::lock_guard lock{active_jobs_mutex_};
  if (--active_jobs_ == 0)
    writer_queue_cv_.notify_one();
  active_jobs_cv_.notify_all();
}
}
