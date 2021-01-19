// Copyright 2020 Kavykin Andrey Kaviandr@yandex.ru

#pragma once

#include <boost/beast/version.hpp>
#include <optional>
#include <string>
#include <utility>
#include <web_crawler_lib/definitions.hpp>

namespace web_crawler_lib {

    using http_response_t = http::response<http::string_body>;

    ::std::optional<::std::pair<::std::string, ::std::string>> parse_url_parts(std::string const& url);

    [[nodiscard]] ::std::optional<http_response_t> read_from_url(::std::string const& host, ::std::string const& port,
                                                                 ::std::string const& target);
}
