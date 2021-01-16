#define QUOTE(...) #__VA_ARGS__
static const char* coreModuleSource = QUOTE(
class Bool {}
class Null {}
class Num {}

class Sequence {
  contains(element) {
    for (var item in this) {
      if (element == item) return true
    }
    return false
  }
}

class List is Sequence {}
class Map is Sequence {}
class Range is Sequence {}

);