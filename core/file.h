// Copyright 2020 Bret Taylor
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "common.h"

namespace compiler {

// A source code file.
class File {
 public:
  static shared_ptr<File> read(const string& path);

  string path;
  string contents;

 private:
  File(const string& path);
};

// The location of a segment of a source code file.
class Location {
 public:
  Location(shared_ptr<File> file, int start_line, int start_column,
           int end_line, int end_column);

  // Returns the union of this location with `other`.
  shared_ptr<Location> merge(shared_ptr<Location> other);

  shared_ptr<File> file;
  int start_line;
  int start_column;
  int end_line;
  int end_column;
};

// Returns the file name portion of the given path.
string file_name(const string& path);

// Returns everything up to file name portion of the given path.
string directory_name(const string& path);

}
