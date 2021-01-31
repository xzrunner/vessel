#define QUOTE(...) #__VA_ARGS__
static const char* coreModuleSource = QUOTE(
class Bool {}
class Null {}
class Num {}
class Null {}

class Sequence 
{
    contains(element) 
    {
        for (var item in this) {
            if (element == item) return true
        }
        return false
    }

    join() { 
        return this.join("") 
    }

    join(sep) 
    {
        var first = true
        var result = ""

        for (var element in this) 
        {
            if (!first) result = result + sep
            first = false
            result = result + element.toString
        }

        return result
    }
}

class List is Sequence {}
class Range is Sequence {}
class String is Sequence {}

class Map is Sequence 
{
    iteratorValue(iterator) 
    {
        return MapEntry(
            this.keyIteratorValue_(iterator),
            this.valueIteratorValue_(iterator)
        )
    }
}

class MapEntry 
{
    init(key, value) {
        this.key = key
        this.value = value
    }
}

class System
{
    static print(obj) {
        this.writeString(obj)
        this.writeString("\n")
	}
}

);