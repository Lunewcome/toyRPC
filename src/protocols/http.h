#ifndef SRC_PROTOCOLS_HTTP_H
#define SRC_PROTOCOLS_HTTP_H

#include <string>
#include <unordered_map>

#include "common/string_util.h"
#include "iobuffer.h"
#include "protocol.h"

class HttpRequest {
 public:
  HttpRequest() = default;
  HttpRequest(IOBuffer* _data) {
    Reset(_data);
  }
  void Swap(HttpRequest* other) {
    method.swap(other->method);
    uri.swap(other->uri);
    version.swap(other->version);
    headers.swap(other->headers);
    content_length = other->content_length;
    content_begin = other->content_begin;
    content_end = other->content_end;
  }
  void Reset(IOBuffer* _data) {
    data = _data;
    method.clear();
    uri.clear();
    version.clear();
    headers.clear();
    content_length = -1;

    opts.caller = this;
    opts.starting_point = data->Begin();
    opts.matcher = &HttpRequest::HitCRFL;
    opts.on_hit = &HttpRequest::OnHitRequestLine;
    opts.on_miss = &HttpRequest::OnMiss;
    opts.on_error = nullptr;
    // DO NOT RESET 'pr'.
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
    // os << "\r\n" << req.content << "\r\n";
    return os;
  }
  // A better name?
  IOBuffer::iterator EndOfRequest() { return content_end; }
  bool GetCallId(uint64_t* call_id) const {
    const auto itrt = headers.find("CallId");
    if (itrt == headers.end()) {
      return false;
    }
    int64_t id;
    bool ret = StringToInt64(itrt->second, &id);
    *call_id = id;
    return ret;
  }
  
  IOBuffer* data;
  std::string method;
  std::string uri;
  std::string version;
  std::unordered_map<std::string, std::string> headers;
  mutable int content_length = -1;
  IOBuffer::iterator content_begin;
  IOBuffer::iterator content_end;

 private:
  friend class Http;
  IOBuffer::ParseOptions opts;
  ParseResult pr;
  // MAKE IT SIMPLE.
  // bool continue_skipping;

  void ForwardStartingPoint();
  void ForwardStartingPoint(IOBuffer::iterator);
  void Finish(IOBuffer::iterator cur);
  static bool HitCRFL(void* _this, IOBuffer::iterator pre,
                      IOBuffer::iterator cur) {
    return *pre == '\r' && *cur == '\n';
  }
  static bool HasCompleteContent(void* _this, IOBuffer::iterator pre,
                                 IOBuffer::iterator cur) {
    auto* req = static_cast<HttpRequest*>(_this);
    return req->content_length == (cur - req->opts.starting_point + 1);
  }
  static void OnHitRequestLine(void* _this, IOBuffer::iterator pre,
                               IOBuffer::iterator cur);
  static void OnHitHeaders(void* _this, IOBuffer::iterator pre,
                           IOBuffer::iterator cur);
  static void OnHitContent(void* _this, IOBuffer::iterator pre,
                           IOBuffer::iterator cur);
  static void OnMiss(void* _this, IOBuffer::iterator pre);
};

struct HttpResponse {
  std::string version;

  int status_code;
  std::string status_text;

  std::unordered_map<std::string, std::string> headers;

  std::string content;
};

class toyRPCController;
class Http { // : public Protocol {
 public:
  Http() {}
  ~Http() {}
  ParseResult Parse(const IOBuffer& data, HttpRequest* req);

  void BuildRequest(const std::string& service_full_name, HttpRequest* req);
  void PackRequest(const HttpRequest& resp, std::string* buf);
  void PackRequest(const toyRPCController& cntl, IOBuffer* out);

  void BuildResponse(const char* msg, int code, HttpResponse* resp);
  void PackResponse(const HttpResponse& resp, std::string* buf);
  void PackResponse(const toyRPCController& cntl, IOBuffer* out);

 private:
};


#endif  // SRC_PROTOCOLS_HTTP_H
