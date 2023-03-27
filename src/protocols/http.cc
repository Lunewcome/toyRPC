#include "http.h"

ParseResult Http::Parse(const IOBuffer& data, HttpRequest* req) {
  while (data.Parse(req->opts)) {
    if (req->pr.GetStatus() == ParseStatus::NEED_MORE_DATA ||
        req->pr.GetStatus() == ParseStatus::OK) {
      break;
    }
  }
  return req->pr;
}

void HttpRequest::OnHitRequestLine(void* _this, IOBuffer::iterator pre,
                                   IOBuffer::iterator cur) {
  auto* req = static_cast<HttpRequest*>(_this);
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
  req->ForwardSavePoint(cur);
}

void HttpRequest::OnHitHeaders(void* _this, IOBuffer::iterator pre,
                               IOBuffer::iterator cur) {
  auto* req = static_cast<HttpRequest*>(_this);
  req->ForwardSavePoint();
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
  req->ForwardSavePoint(cur);
}

void HttpRequest::OnHitContent(void* _this, IOBuffer::iterator pre,
                               IOBuffer::iterator cur) {
  auto* req = static_cast<HttpRequest*>(_this);
  req->ForwardSavePoint();
  req->content_begin = req->opts.starting_point;
  req->content_end = cur;
  req->Finish(cur);
}

void HttpRequest::OnMiss(void* _this, IOBuffer::iterator pre) {
  auto* req = static_cast<HttpRequest*>(_this);
  req->pr.SetStatus(ParseStatus::NEED_MORE_DATA);
}

void HttpRequest::ForwardSavePoint() {
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

void HttpRequest::ForwardSavePoint(IOBuffer::iterator cur) {
  cur = data->Skip(cur, '\n');
  pr.SetStatus(*cur == '\n' ?
      ParseStatus::NEED_MORE_DATA : ParseStatus::CONTINUE);
  opts.starting_point = cur;
}

void HttpRequest::Finish(IOBuffer::iterator cur) {
  data->PopFront(cur);
  pr.SetStatus(ParseStatus::OK);
  opts.starting_point = cur;
  opts.matcher = &HttpRequest::HitCRFL;
  opts.on_hit = &HttpRequest::OnHitRequestLine;
  // opts.on_miss = &HttpRequest::OnMiss;
  // opts.on_error = nullptr;
}

void Http::PackRequest(const HttpResponse& resp, std::string* buf) {
  StringAppendF(buf, "%s %d %s\r\n",
                resp.version.c_str(),
                resp.status_code,
                resp.status_text.c_str());
  for (const auto& ele : resp.headers) {
    StringAppendF(buf, "%s: %s\r\n", ele.first.c_str(), ele.second.c_str());
  }
  StringAppendF(buf, "\r\n%s", resp.content.c_str());
}
