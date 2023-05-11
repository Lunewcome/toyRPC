#include "http.h"

#include "controller.h"

ParseResult Http::Parse(const IOBuffer& data, HttpRequest* req) {
  while (data.Parse(req->opts)) {
    if (req->pr.GetStatus() == ParseStatus::NEED_MORE_DATA ||
        req->pr.GetStatus() == ParseStatus::OK ||
        req->pr.GetStatus() == ParseStatus::UNIMPLEMENTED) {
      break;
    }
  }
  return req->pr;
}

void HttpRequest::OnHitRequestLine(void* _this, IOBuffer::iterator pre,
                                   IOBuffer::iterator cur) {
  auto* req = static_cast<HttpRequest*>(_this);
  // Do not need to forward starting point for a new request.
  // [starting_point, pre)
  const auto& request_line = req->data->AsString(req->opts.starting_point, pre);
  std::vector<std::string> splits;
  SplitString(request_line, ' ', &splits);
  CHECK_EQ(splits.size(), 3);
  req->method.swap(splits[0]);
  req->uri.swap(splits[1]);
  req->version.swap(splits[2]);
  // update opts.
  req->opts.on_hit = &HttpRequest::OnHitHeaders;
  req->ForwardStartingPoint(cur);
}

void HttpRequest::OnHitHeaders(void* _this, IOBuffer::iterator pre,
                               IOBuffer::iterator cur) {
  auto* req = static_cast<HttpRequest*>(_this);
  req->ForwardStartingPoint();
  if (req->opts.starting_point == pre) {
    // end of headers.
    req->content_length = req->ContentLength();
    if (req->content_length == 0) {
      req->Finish(cur);
      return;
    }
    req->opts.matcher = &HttpRequest::HasCompleteContent;
    req->opts.on_hit = &HttpRequest::OnHitContent;
  } else {
    std::vector<std::string> splits;
    // [req->opts.starting_point, pre)
    const std::string& header =
        req->data->AsString(req->opts.starting_point, pre);
    SplitString(header, ':', &splits);
    if (splits.size() >= 3) {
      std::string tmp;
      for (size_t i = 1; i < splits.size(); ++i) {
        StringAppendF(&tmp, i == 1 ? "%s" : ":%s", splits[i].c_str());
      }
      req->headers[splits[0]] = tmp;
    } else if (splits.size() == 2) {
      req->headers[splits[0]] = splits[1];
    } else {
      CHECK(false) << "What happed:" << header;
    }
  }
  req->ForwardStartingPoint(cur);
}

void HttpRequest::OnHitContent(void* _this, IOBuffer::iterator pre,
                               IOBuffer::iterator cur) {
  auto* req = static_cast<HttpRequest*>(_this);
  req->ForwardStartingPoint();
  req->content_begin = req->opts.starting_point;
  req->content_end = cur;
  req->Finish(cur);
}

void HttpRequest::OnMiss(void* _this, IOBuffer::iterator pre) {
  auto* req = static_cast<HttpRequest*>(_this);
  req->pr.SetStatus(ParseStatus::NEED_MORE_DATA);
}

void HttpRequest::ForwardStartingPoint() {
  if (pr.GetStatus() == ParseStatus::NEED_MORE_DATA) {
    // parsing a request.
    opts.starting_point = data->Skip(opts.starting_point, '\n');
    CHECK(*opts.starting_point != '\n');
  } else if (pr.GetStatus() == ParseStatus::OK) {
    // previous request has been parsed, prepare for the next one.
    ++opts.starting_point;
    CHECK(opts.starting_point.IsValid());
  } else {
    //?
  }
}

void HttpRequest::ForwardStartingPoint(IOBuffer::iterator cur) {
  cur = data->Skip(cur, '\n');
  pr.SetStatus(*cur == '\n' ?
      ParseStatus::NEED_MORE_DATA : ParseStatus::CONTINUE);
  opts.starting_point = cur;
}

void HttpRequest::Finish(IOBuffer::iterator cur) {
  data->PopFront(cur);
  if (IsGet() || IsPost()) {
    pr.SetStatus(ParseStatus::OK);
  } else {
    pr.SetStatus(ParseStatus::UNIMPLEMENTED);
  }
  opts.starting_point = cur;
  opts.matcher = &HttpRequest::HitCRFL;
  opts.on_hit = &HttpRequest::OnHitRequestLine;
  // opts.on_miss = &HttpRequest::OnMiss;
  // opts.on_error = nullptr;
}

void Http::BuildRequest(const std::string& service_full_name, HttpRequest* req) {
  req->method = "GET";
  req->version = "HTTP/1.1";
  req->headers["Content-Length"] = "";
  req->headers["ServiceFullName"] = "";
}

void Http::BuildResponse(const char* msg, int code, HttpResponse* resp) {
  resp->version = "HTTP/1.1";
  resp->status_code = code;
  resp->status_text = "Not Found";
  resp->content.append(msg, strlen(msg));
  resp->content.append("\n", 1);

  std::string sz;
  StringPrintf(&sz, "%d", resp->content.size());
  resp->headers["Content-Length"] = sz;
  resp->headers["Content-Type"] = "text/plain";
}

void Http::PackResponse(const HttpResponse& resp, std::string* buf) {
  StringAppendF(buf, "%s %d %s\r\n",
                resp.version.c_str(),
                resp.status_code,
                resp.status_text.c_str());
  for (const auto& ele : resp.headers) {
    StringAppendF(buf, "%s: %s\r\n", ele.first.c_str(), ele.second.c_str());
  }
  StringAppendF(buf, "\r\n%s", resp.content.c_str());
}

void Http::PackResponse(const toyRPCController& cntl, IOBuffer* out) {
  VLOG(1) << "To improve this code later...";
  std::string buf;
  PackResponse(cntl.http_response, &buf);
  out->Append(buf);
}

void Http::PackRequest(const HttpRequest& req, std::string* buf) {
  StringAppendF(buf, "GET / HTTP/1.1\r\n");
  for (const auto& ele : req.headers) {
    StringAppendF(buf, "%s: %s\r\n", ele.first.c_str(), ele.second.c_str());
  }
}

void Http::PackRequest(const toyRPCController& cntl, IOBuffer* out) {
  VLOG(1) << "To improve this code later...";
  std::string buf;
  PackRequest(cntl.http_request, &buf);
  out->Append(buf);
}
