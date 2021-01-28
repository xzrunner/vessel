#include "utility.h"

#include <vessel.h>

#include <string>

namespace
{

#define OUT_BUF_SIZE 1024

char* out_buf = NULL;
size_t out_ptr = 0;

void write(const char* text)
{
    if (strcmp(text, "\\n") == 0) {
        out_ptr += sprintf(&out_buf[out_ptr], "\n");
    } else {
        out_ptr += sprintf(&out_buf[out_ptr], text);
    }
}

}

void config_vm()
{
    VesselConfiguration cfg;
    cfg.load_module_fn = nullptr;
    cfg.bind_foreign_class_fn = nullptr;
    cfg.bind_foreign_method_fn = nullptr;
    cfg.write_fn = write;
    ves_set_config(&cfg);
}

void init_output_buf()
{
	if (out_buf) {
		out_buf[0] = 0;
	}
	out_ptr = 0;

	if (out_buf == NULL) {
        out_buf = new char[OUT_BUF_SIZE];
		memset(out_buf, 0, OUT_BUF_SIZE);
	}
}

const char* get_output_buf()
{
    return out_buf;
}