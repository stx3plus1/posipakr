/*
 * posipakx - header library for r/w on posipak files
 * by stx4
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	char validation[4];
	unsigned long file_size;
	unsigned long data_offset;
	char filename[32];
} pak_file;

typedef struct {
	char validation[4];
	unsigned long data_size;
	unsigned char lookup_len;
	unsigned long data_offset;
} posipak_header;

typedef struct {
	posipak_header header;
	pak_file* lookup;
	char* data;
} posipak;

char* signature = "POSI";
char* file_signature = "PAKF";

posipak* create_posipak(void) {
	posipak* pak = malloc(sizeof(posipak));
	if (!pak) return NULL; /* malloc skill issue */

	memcpy(&pak->header.validation, signature, 4);

	pak->data   = NULL;
	pak->lookup = NULL;

	pak->header.data_size = 0;
	pak->header.lookup_len = 0;
	pak->header.data_offset = sizeof(posipak_header);

	return pak;
}

posipak* load_posipak(const char* filename) {
	if (!filename) return NULL;

	FILE* pak_read = fopen(filename, "rb");
	if (!pak_read) return NULL;

	fseek(pak_read, 0, SEEK_END);
	long read_len = ftell(pak_read);
	fseek(pak_read, 0, SEEK_SET);

	char* pak_buf = malloc(read_len);
	if (!pak_buf) {
		fclose(pak_read);
		return NULL;
	}
	fread(pak_buf, read_len, 1, pak_read);
	fclose(pak_read);

	posipak* pak = malloc(sizeof(posipak));
	memcpy(&pak->header, pak_buf, sizeof(posipak_header));
	if (memcmp(pak->header.validation, signature, 4)) { /* failed validation */
		free(pak_buf);
		free(pak);
		return NULL;
	}
	
	pak->data   = malloc(pak->header.data_size);
	pak->lookup = malloc(sizeof(pak_file) * pak->header.lookup_len);

	memcpy(pak->lookup, pak_buf + sizeof(posipak_header), sizeof(pak_file) * pak->header.lookup_len);
	memcpy(pak->data, pak_buf + (sizeof(posipak_header) + (sizeof(pak_file) * pak->header.lookup_len)), pak->header.data_size);
	
	free(pak_buf);
	return pak;
}

void* pak_load_file(posipak* pak, const char* name, long* len) {
	pak_file* file = NULL;

	for (int i = 0; i < pak->header.lookup_len; i++) {
		if (memcmp(pak->lookup[i].validation, file_signature, 4)) /* theres a corrupt/misaligned file in pak */
			return NULL;
		if (strncmp(pak->lookup[i].filename, name, sizeof(pak->lookup[i].filename)) == 0) {
			file = &pak->lookup[i];
			break;
		}
	}
	if (!file) return NULL;

	void* file_buf = malloc(file->file_size);
	if (!file_buf) return NULL;
	memcpy(file_buf, pak->data + file->data_offset, file->file_size);
	*len = file->file_size;
	return file_buf;
}

void pak_add_file(posipak* pak, const char* name, void* data, long size) {
	if (!pak || !name || !data || size <= 0) return;

	long payload_offset = pak->header.data_size;

	pak_file* new_lookup = realloc(pak->lookup, sizeof(pak_file) * (pak->header.lookup_len + 1));
	if (!new_lookup) return; /* do you have 512 KiB of memory or are you using all of it */
	pak->lookup = new_lookup;

	pak_file* new_entry = &pak->lookup[pak->header.lookup_len];
	memcpy(new_entry->validation, file_signature, 4);
	strncpy(new_entry->filename, name, sizeof(new_entry->filename) - 1);
	new_entry->filename[31] = '\0';
	new_entry->file_size = size;
	new_entry->data_offset = payload_offset;

	pak->header.lookup_len++;

	char* new_data = realloc(pak->data, pak->header.data_size + size);
	if (!new_data) return; /* ditto last comment */
	pak->data = new_data;

	memcpy(pak->data + payload_offset, data, size);
	pak->header.data_size += size;
}

int pak_delete_file(posipak* pak, const char* name) {
	int index;
	pak_file* file = NULL;
	pak_file petrif;

	for (int i = 0; i < pak->header.lookup_len; i++) {
		if (memcmp(pak->lookup[i].validation, file_signature, 4))
			continue; /* invalid PAKF signature of file? */
		if (strncmp(pak->lookup[i].filename, name, sizeof(pak->lookup[i].filename)) == 0) {
			file = &pak->lookup[i];
			index = i;
			break;
		}
	}
	if (!file) return 1;
	petrif = *file;

	memmove(&pak->lookup[index], &pak->lookup[index + 1], (pak->header.lookup_len - index - 1) * sizeof(pak_file));
	pak->header.lookup_len--;

	if (pak->header.lookup_len < 1) {
		free(pak->lookup);
		pak->lookup = NULL;
	} else {
		pak_file* new_lookup = realloc(pak->lookup, sizeof(pak_file) * (pak->header.lookup_len));
		if (!new_lookup) {
			free(pak->lookup);
			pak->lookup = NULL;
			return 1;
		}
		pak->lookup = new_lookup;
	}

	long int end = petrif.data_offset + petrif.file_size;
	memmove(&pak->data[petrif.data_offset], &pak->data[end], pak->header.data_size - end);

	pak->header.data_size -= petrif.file_size;

	if (pak->header.data_size < 1) {
		free(pak->data);
		pak->data = NULL;
	} else {
		void* new_data = realloc(pak->data, pak->header.data_size);
		if (!new_data) {
			free(pak->data);
			pak->data = NULL;
			return 1;
		}
		pak->data = new_data;
	}

	for (int i = index; i < pak->header.lookup_len; i++) {
		pak->lookup[i].data_offset -= petrif.file_size;
	}

	return 0;
}

void write_posipak(posipak* pak, const char* filename) {
	if (!pak || !filename) return;

	FILE* pak_write = fopen(filename, "wb");
	if (!pak_write) return;

	fwrite(&pak->header, sizeof(posipak_header), 1, pak_write);
	if (pak->lookup) fwrite(pak->lookup, sizeof(pak_file), pak->header.lookup_len, pak_write);
	if (pak->data)   fwrite(pak->data, pak->header.data_size, 1, pak_write);
	fclose(pak_write);
}

void destroy_posipak(posipak* pak) {
	if (pak->data)   free(pak->data);
	if (pak->lookup) free(pak->lookup);
	if (pak)         free(pak);
}

#ifdef __cplusplus
}
#endif