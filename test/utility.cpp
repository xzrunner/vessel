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

void fill_config(VesselConfiguration* cfg,
                 VesselBindForeignClassFn bind_class,
                 VesselBindForeignMethodFn bind_method)
{
    // Value-initialize the WHOLE struct: ves_set_config() memcpy's it over
    // the VM's copy, so leaving a field (e.g. expand_modules_fn)
    // uninitialized would graft garbage stack bytes into the live config.
    *cfg = VesselConfiguration{};
    cfg->load_module_fn = nullptr;
    cfg->bind_foreign_class_fn = bind_class;
    cfg->bind_foreign_method_fn = bind_method;
    cfg->write_fn = write;
}

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

void config_vm()
{
    VesselConfiguration cfg;
    fill_config(&cfg, nullptr, nullptr);
    ves_set_config(&cfg);
}

// Test-only: like config_vm(), but wires the foreign class/method bind
// callbacks so test/foreign.cpp can exercise the foreign machinery while
// keeping the shared output-buffer write_fn. Declared locally in
// test/foreign.cpp (utility.h stays untouched).
void config_foreign_vm(VesselBindForeignClassFn bind_class,
                       VesselBindForeignMethodFn bind_method)
{
    VesselConfiguration cfg;
    fill_config(&cfg, bind_class, bind_method);
    ves_set_config(&cfg);
}
