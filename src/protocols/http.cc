#include "protocols/http.h"

ParseResult Http::Parse(IOBuffer& data, HttpRequest* req) {
  ParseResult pr;
  while (!data.Empty()) {
    if (req->state == HttpRequest::ParseState::REQUEST_LINE ||
        req->state == HttpRequest::ParseState::HEADERS) {
      int pos = data.Find('\n');
      pr.SetStatus(ParseStatus::NEED_MORE_DATA);
      if (pos == -1) {
        return pr;
      }
      pos = data.GetPrePosition(pos);
      CHECK(data[pos] == '\r');
      std::vector<std::string> splits;
      if (req->state == HttpRequest::ParseState::REQUEST_LINE) {
          data.Split(pos, ' ', &splits);
          CHECK_EQ(splits.size(), 3);
          req->method.swap(splits[0]);
          req->uri.swap(splits[1]);
          req->version.swap(splits[2]);
          req->state = HttpRequest::ParseState::HEADERS;
      } else { // req->state == HttpRequest::ParseState::HEADERS)
          data.Split(pos, ':', &splits);
          if (splits.size() >= 3) {
            std::string tmp;
            for (size_t i = 1; i < splits.size(); ++i) {
              if (i == 1) {
                StringAppendF(&tmp, "%s", splits[i].c_str());
              } else {
                StringAppendF(&tmp, ":%s", splits[i].c_str());
              }
            }
            req->headers[splits[0]] = tmp;
          } else if (splits.size() == 2) {
            req->headers[splits[0]] = splits[1];
          } else if (splits.size() == 0) {
            req->state = (req->ContentLength() > 0) ?
                HttpRequest::ParseState::CONTENT : HttpRequest::ParseState::DONE;
          } else {
            CHECK(false) << "what happened?";
          }
      }
      // data[pos, pos + 1] is now '\r\n'. Seek to the next one to '\n'.
      data.Confirm(pos + 2);
    } else if (req->state == HttpRequest::ParseState::CONTENT) {
      int content_length = req->ContentLength();
      if (content_length > data.Size()) {
        pr.SetStatus(ParseStatus::NEED_MORE_DATA);
      } else {
        data.MoveData(content_length, &req->content);
        pr.SetStatus(ParseStatus::OK);
      }
      break;
    } else {
      CHECK(false) << "should not be here...";
    }
    // finally...
    if (req->state == HttpRequest::ParseState::DONE) {
      // accept 'GET' and 'POST' only.
      if (!req->IsGet() && !req->IsPost()) {
        pr.SetStatus(ParseStatus::UNIMPLEMENTED);
      } else {
        pr.SetStatus(ParseStatus::OK);
      }
      break;
    }
  }
  return pr;
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
