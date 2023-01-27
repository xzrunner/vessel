#include "opt_io.h"

#if OPT_IO

#include "opt_io.ves.inc"
#include "object.h"

#include <stdio.h>
#include <stdlib.h> // for malloc

typedef struct 
{
	FILE* fp;
    bool binary;
} File;

static void w_File_allocate()
{
	File* file = (File*)ves_set_newforeign(0, 0, sizeof(File));
	file->fp = NULL;
}

static int file_finalize(void* data)
{
    return sizeof(File);
}

void w_File_open()
{
    File* file = (File*)ves_toforeign(0);
    const char* filename = ves_tostring(1);
    const char* mode = ves_tostring(2);
    if (file->fp) {
        fclose(file->fp);
    }

    file->fp = fopen(filename, mode);
    if (!file->fp) {
        perror("fopen");
    }

    file->binary = false;
    for (int i = 0, n = strlen(mode); i < n; ++i) {
        if (mode[i] == 'b') {
            file->binary = true;
            break;
        }
    }
}

void w_File_close()
{
    File* file = (File*)ves_toforeign(0);
    if (file->fp) {
        fclose(file->fp);
    }
}

void w_File_read()
{
    File* file = (File*)ves_toforeign(0);
    if (!file->fp) {
        ves_set_nil(0);
        return;
    }

    fseek(file->fp, 0, SEEK_END);
    size_t size = ftell(file->fp);
    rewind(file->fp);

    char* buffer = (char*)malloc(sizeof(char) * (size + 1));
    if (buffer == NULL) { 
        return; 
    }
    buffer[size] = 0;

    size_t result = fread(buffer, 1, size, file->fp);
    if (result > size) {
        free(buffer);
        fclose(file->fp);
        return;
    }
    buffer[result] = 0;

    ves_set_lstring(0, buffer, result);
    free(buffer);
}

void w_File_write()
{
    File* file = (File*)ves_toforeign(0);
    if (!file->fp) {
        return;
    }

    const char* str = ves_tostring(1);
    if (file->binary) {
        fwrite(str, sizeof(str[0]), sizeof(str) / sizeof(str[0]), file->fp);
    } else {
        fprintf(file->fp, str);
    }
}

void w_File_is_valid()
{
    File* file = (File*)ves_toforeign(0);
    ves_set_boolean(0, file->fp ? true : false);
}

const char* IOSource()
{
	return ioModuleSource;
}

VesselForeignClassMethods IOBindForeignClass(const char* module, const char* class_name)
{
    VesselForeignClassMethods methods;
    methods.allocate = NULL;
    methods.finalize = NULL;

    if (strcmp(class_name, "File") == 0)
    {
        VesselForeignClassMethods methods;
        methods.allocate = w_File_allocate;
        methods.finalize = file_finalize;
        return methods;
    }

    return methods;
}

VesselForeignMethodFn IOBindForeignMethod(const char* class_name, bool is_static, const char* signature)
{
    if (strcmp(class_name, "File") == 0)
    {
        if (strcmp(signature, "<allocate>") == 0) return w_File_allocate;
        if (strcmp(signature, "open(_,_)") == 0) return w_File_open;
        if (strcmp(signature, "close()") == 0) return w_File_close;
        if (strcmp(signature, "read()") == 0) return w_File_read;
        if (strcmp(signature, "write(_)") == 0) return w_File_write;
        if (strcmp(signature, "is_valid()") == 0) return w_File_is_valid;
    }

    return NULL;
}

#endif