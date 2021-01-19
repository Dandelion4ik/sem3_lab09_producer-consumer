// Copyright 2020 Kavykin Andrey Kaviandr@yandex.ru

#pragma once

#include <iostream>
#include <set>
#include <string>
#include <web_crawler_lib/parser.hpp>
#include <web_crawler_lib/url_reader.hpp>

namespace web_crawler_lib {

    struct ParsingResult {
        ::std::set<::std::string> image_urls;
        ::std::set<::std::string> child_urls;

        [[nodiscard]] bool operator ==(ParsingResult const& other) const;
    };

    ::std::ostream &operator<<(::std::ostream &out, ParsingResult const& result);

    [[nodiscard]] ParsingResult parse_http_response(http_response_t const& response, bool parse_children);
}
