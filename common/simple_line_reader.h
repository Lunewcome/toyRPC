/** 
* @brief    simple line reader.
* @date     2014-05-14
* @Authoer  XinlaiLu(luxinlai@baidu.com), Hi(newcome_lu)
*/
#ifndef BASE_SIMPLE_LINE_READER_H_
#define BASE_SIMPLE_LINE_READER_H_
#include "common/basics.h"
#include "common/callback.h"

#include <cstdlib>

#include <fstream>
#include <vector>
#include <string>
using std::ifstream;
using std::string;
using std::vector;

class SimpleLineReader {
 public:
  explicit SimpleLineReader(const string& path,
                            bool skip_empty = true)
      : file_name_(path),
        skip_empty_(skip_empty) {
    Open();
    opened_ = true;
  }
  ~SimpleLineReader() {}
  // Read into buffer one time!
  void ReadLines(std::vector<std::string>* lines);
  void AppendLinesToString(string* buf);
  // callback.
  void ProcessLines(Closure<string>* closure);

  // one line a time.
  inline bool HasNext() {
    return getline(file_, line_buf_);
  }
  inline const string& Get() const {
    return line_buf_;
  }

 private:
  void Open() {
    file_.open(file_name_.c_str());
    if (!file_.is_open()) {
      abort();
    }
  }

  const string file_name_;
  bool skip_empty_;
  ifstream file_;
  string line_buf_;
  bool opened_;
  DO_NOT_COPY_AND_ASSIGN(SimpleLineReader);
};

#endif  // BASE_SIMPLE_LINE_READER_H_
