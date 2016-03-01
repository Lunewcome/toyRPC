/**
*/
#include "common/simple_line_reader.h"

void SimpleLineReader::ReadLines(
    vector<string>* lines) {
  while (getline(file_, line_buf_)) {
    if (line_buf_.empty() && skip_empty_) {
      continue;
    }
    lines->push_back(line_buf_);
  }
}

void SimpleLineReader::AppendLinesToString(
    string* buf) {
  vector<string> lines;
  ReadLines(&lines);
  for (size_t i = 0; i < lines.size(); ++i) {
    *buf += lines[i];
  }
}

void SimpleLineReader::ProcessLines(
    Closure<string>* closure) {
  while (getline(file_, line_buf_)) {
    if (line_buf_.empty() && skip_empty_) {
      continue;
    }
    closure->Run(line_buf_);
  }
}
