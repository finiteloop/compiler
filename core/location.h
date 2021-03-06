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

// A position within a source file.
struct Position {
  shared_ptr<filesystem::path> path;
  size_t line = 1;
  size_t column = 1;
};

// The location of a sequence of characters in a source file.
struct Location {
  Position begin;
  Position end;
};

}
