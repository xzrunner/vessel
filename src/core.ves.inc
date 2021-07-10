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
            result = result + element.toString()
        }

        return result
    }
}

class List is Sequence 
{
    toString()
    {
        var first = true
        var result = "[ "

        for (var element in this) 
        {
            if (!first) result = result + ", "
            first = false
            if (element is String) {
                result = result + "\"" + element.toString() + "\""
            } else {
                result = result + element.toString()
            }
        }

        result = result + " ]"

        return result
    }

    remove(element)
    {
        var break = false
        for (var i = 0; i < this.count and !break; i = i + 1) {
            if (this[i] == element) {
                this.removeAt(i)
                break = true
            }
        }
    }

    pop_front()
    {
		if (!this.isEmpty) {
			this.removeAt(0)
		}
    }

    pop_back()
    {
		if (!this.isEmpty) {
			this.removeAt(this.count - 1)
		}
    }

    query_index(value)
    {
		for (var i = 0; i < this.count; i = i + 1) {
			if (this[i] == value) {
				return i
			}
		}
		return -1
    }
}

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

    toString()
    {
        var first = true
        var result = "{ "

        for (var element in this) 
        {
            if (!first) result = result + ", "
            first = false
            if (element.key is String) {
                result = result + "\"" + element.key.toString() + "\""
            } else {
                result = result + element.key.toString()
            }
            result = result + " : "
            if (element.value is String) {
                result = result + "\"" + element.value.toString() + "\""
            } else {
                result = result + element.value.toString()
            }
        }

        result = result + " }"

        return result
    }
}

class Set is Sequence 
{
    toString()
    {
        var first = true
        var result = "< "

        for (var element in this) 
        {
            if (!first) result = result + ", "
            first = false
            if (element is String) {
                result = result + "\"" + element.toString() + "\""
            } else {
                result = result + element.toString()
            }
        }

        result = result + " >"

        return result
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

class Basic
{
    static loadstring(str) {}
}

);