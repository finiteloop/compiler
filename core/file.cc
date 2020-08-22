// Copyright 2020 Bret Taylor

#include "file.h"

#include <filesystem>
#include <fstream>

namespace compiler {

File::File(const string& path) : path(path) {
}

shared_ptr<File> File::read(const string& path) {
  std::ifstream file(path.c_str());
  if (file) {
    shared_ptr<File> result(new File(path));
    file.seekg(0, std::ios::end);
    result->contents.resize(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(&result->contents[0], result->contents.size());
    if (file.fail()) {
      return nullptr;
    }
    return result;
  } else {
    return nullptr;
  }
}

Location::Location(shared_ptr<File> file, int start_line, int start_column,
                   int end_line, int end_column)
    : file(file),
      start_line(start_line),
      start_column(start_column),
      end_line(end_line),
      end_column(end_column) {
}

shared_ptr<Location> Location::merge(shared_ptr<Location> other) {
  shared_ptr<Location> merged(
      new Location(file, start_line, start_column, end_line, end_column));
  if (other->start_line < start_line ||
      (other->start_line == start_line && other->start_column < start_column)) {
    merged->start_line = other->start_line;
    merged->start_column = other->start_column;
  }
  if (other->end_line > end_line ||
      (other->end_line == end_line && other->end_column > end_column)) {
    merged->end_line = other->end_line;
    merged->end_column = other->end_column;
  }
  return merged;
}

string file_name(const string& path) {
  auto pos = path.rfind(std::filesystem::path::preferred_separator);
  if (pos == string::npos || pos + 1 == path.size()) {
    return path;
  }
  return path.substr(pos + 1);
}

string directory_name(const string& path) {
  auto pos = path.rfind(std::filesystem::path::preferred_separator);
  if (pos == string::npos || pos + 1 == path.size()) {
    return path;
  }
  return path.substr(0, pos);
}

}
