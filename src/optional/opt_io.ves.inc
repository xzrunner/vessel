#define QUOTE(...) #__VA_ARGS__
static const char* ioModuleSource = QUOTE(

foreign class File {
	init() {}
	foreign open(filename, mode)
	foreign close()
	foreign read()
	foreign write(str)
	foreign is_valid()
}

foreign class IO {
	static open(filename, mode) {
		var f = File.init()
		f.open(filename, mode)
		return f
	}

	static close(file) {
		file.close()
	}
}
);
