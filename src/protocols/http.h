#ifndef SRC_PROTOCOLS_HTTP_H
#define SRC_PROTOCOLS_HTTP_H

#include <ostream>
#include <string>
#include <unordered_map>

#include "protocol.h"
#include "common/string_util.h"

struct HttpRequest {
  enum class ParseState {
    REQUEST_LINE = 1,
    HEADERS = 2,
    CONTENT = 3,

    DONE = 10,
  };

  void Reset() {
    state = ParseState::REQUEST_LINE;
    method.clear();
    uri.clear();
    version.clear();
    headers.clear();
    content_length = -1;
    content.clear();
  }
  bool IsGet() const {
    return method == "GET" || method == "get";
  }
  bool IsPost() const {
    return method == "POST" || method == "post";
  }
  int ContentLength() const {
    if (content_length < 0) {
      const auto& itrt = headers.find("Content-Length");
      if (itrt == headers.end()) {
        content_length = 0;
      } else {
        content_length = StringToInt(itrt->second);
      }
    }
    return content_length;
  }
  friend std::ostream& operator<<(std::ostream& os, const HttpRequest& req) {
    os << req.method << " " << req.uri << " " << req.version << "\r\n";
    for (const auto& ele : req.headers) {
      os << ele.first << ":" << ele.second << "\r\n";
    }
    os << "\r\n" << req.content << "\r\n";
    return os;
  }

  ParseState state = ParseState::REQUEST_LINE;
  std::string method;
  std::string uri;
  std::string version;
  std::unordered_map<std::string, std::string> headers;
  mutable int content_length = -1;
  std::string content;
};

struct HttpResponse {
  std::string version;

  int status_code;
  std::string status_text;

  std::unordered_map<std::string, std::string> headers;

  std::string content;
};

class Http { // : public Protocol {
 public:
  Http() {}
  ~Http() {}
  ParseResult Parse(IOBuffer& data, HttpRequest* req);
  void PackRequest(const HttpResponse& resp, std::string* buf);

 private:
};

#endif  // SRC_PROTOCOLS_HTTP_H
