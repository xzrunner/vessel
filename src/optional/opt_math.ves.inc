#define QUOTE(...) #__VA_ARGS__
static const char* mathModuleSource = QUOTE(
class Math 
{
	foreign static abs(num)
	foreign static acos(num)
	foreign static asin(num)
	foreign static atan(num)
	foreign static atan2(y, x)
	foreign static ceil(num)
	foreign static cos(num)
	foreign static floor(num)
	foreign static negate(num)
	foreign static round(num)
	foreign static sin(num)
	foreign static sqrt(num)
	foreign static tan(num)
	foreign static log(num)
	foreign static log2(num)
	foreign static exp(num)
	foreign static pi()
	foreign static min(x, y)
	foreign static max(x, y)
	foreign static clamp(num, min, max)
	foreign static pow(x, y)
	foreign static mod(x, y)

	static radians(degrees) {
		return degrees * 0.01745329251994329576923690768489
	}
}
);
