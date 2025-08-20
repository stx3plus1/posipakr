/*
 * posipakr
 * by stx4
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "posipakx.h"

#define destroy_quit(exit) destroy_posipak(pak); return exit;
#define bad_pak_helper(pakf) \
if (memcmp(pakf->validation, file_signature, 4)) { \
	puts("[x] bad file found in pak :<"); \
	continue; \
}
#define arg_count(count) \
if (options.file_amt < count) { \
	puts("[x] not enough arguments >:/"); \
	print_help(); \
	destroy_quit(1); \
} \
else if (!f[options.file_amt - 1]) { \
	puts("[x] missing some file args!?"); \
	print_help(); \
	destroy_quit(1); \
}

typedef struct {
	int add;
	int list;
	int dump;
	int create;
	int delete;
	int extract;
	int file_amt;
} pakr_options;
char* f[3] = {0};

const char* help[] = {
	"",
	"posipakr 1.0, by stx4",
	"",
	"options:",
	"	-c: new posipak  (pak)",
	"	-l: list files   (pak)",
	"	-e: extract all  (pak)",
	"	-r: delete file  (pak, pakname)",
	"	-a: add file     (pak, filename, pakname)",
	"	-d: dump file    (pak, filename, pakname)",
	"\n"
	"don't even try mix same-count argument cmds",
	"(back up your pak, unless you like chaos)",
	"",
	"returning you to your freedom..."
};

void print_help(void) {
	for (int i = 0; i < sizeof(help) / sizeof(help[0]); i++)
		puts(help[i]);
}

int main(int ac, char* av[]) {
	if (ac < 2) {
		puts("[i] no options provided");
		print_help();
		return 0;
	}

	pakr_options options = { 0 };
	int j = 0;
	for (int i = 1; i < ac; i++) {
		if (av[i][0] == '-') {
			for (char* p = (av[i] + 1); *p; p++) {
				switch (*p) {
				case 'c': options.create  = 1; break;
				case 'l': options.list    = 1; break;
				case 'e': options.extract = 1; break;
				case 'r': options.delete  = 1; break;
				case 'a': options.add     = 1; break;
				case 'd': options.dump    = 1; break;

				default: printf("[i] unknown option: %c\n", *p); break;
				}
			}
		} else {
			if (j < 3) f[j++] = av[i], options.file_amt = j + 1;
			else printf("[i] ignoring unusable argument \"%s\"\n", av[i]);
		}
	}

	if (options.file_amt < 1) {
		puts("[x] perform operations on... what? provide a file >:(");
		print_help();
		return 1;
	}

	posipak* pak = NULL;
	if (options.create) {
		pak = create_posipak(); 
		puts("[i] created new pak");
	} else {
		pak = load_posipak(f[0]);
		if (!pak) {
			puts("[x] invalid file or pak signature :c");
			return 1;
		}
	}

	if (options.add) {
		arg_count(3);

		long file_len;
		FILE* load_file;

		load_file = fopen(f[1], "rb");
		if (!load_file) {
			puts("[x] failed to open file (does it exist?) :c");
			destroy_quit(1);
		}

		fseek(load_file, 0, SEEK_END);
		file_len = ftell(load_file);
		fseek(load_file, 0, SEEK_SET);

		void* file_data = malloc(file_len);
		if (!file_data) {
			puts("[x] memory allocation fail!? :'c");
			fclose(load_file);
			destroy_quit(1);
		}

		if (fread(file_data, 1, file_len, load_file) != file_len) {
			puts("[x] failed to read file :c");
			fclose(load_file);
			free(file_data);
			destroy_quit(1);
		}
		fclose(load_file);

		pak_add_file(pak, f[2], file_data, file_len);
		free(file_data);
		printf("[i] added %s as %s to pak\n", f[1], f[2]);
	}

	if (options.delete) {
		int ret = pak_delete_file(pak, f[1]);
		if (ret) {
			puts("[x] failed to delete file (does it exist in-pak?) :c");
			destroy_quit(1);
		}
		printf("[i] deleted %s from pak\n", f[1]);
	}

	if (options.list) {
		if (pak->header.lookup_len == 0) {
			puts("[i] there's nothing in there");
		} else {
			for (int i = 0; i < pak->header.lookup_len; i++) {
				pak_file* cur = &pak->lookup[i];
				bad_pak_helper(cur);

				printf("[i] %s - %lu bytes\n", cur->filename, cur->file_size);
			}
		}
	}
	
	if (options.dump) {
		arg_count(3);

		long file_len;
		FILE* dump_file;
		char* file_data = NULL;

		dump_file = fopen(f[1], "wb");
		if (!dump_file) {
			puts("[x] failed to open output file");
		}

		file_data = pak_load_file(pak, f[2], &file_len);
		if (!file_data) {
			puts("[x] failed to read pak file data (does it exist in-pak?) :c");
			destroy_quit(1);
		}

		if (fwrite(file_data, 1, file_len, dump_file) != file_len) {
			puts("[x] failed to write output file (check permissions, free space?) :c");
			fclose(dump_file);
			free(file_data);
			destroy_quit(1);
		}
		fclose(dump_file);
		free(file_data);

		printf("[i] dumped %s to %s\n", f[2], f[1]);
	}
	
	if (options.extract) {
		long file_len;
		FILE* dump_file;
		char* file_data = NULL;

		for (int i = 0; i < pak->header.lookup_len; i++) {
			pak_file* cur = &pak->lookup[i];
			bad_pak_helper(cur);

			file_data = pak_load_file(pak, cur->filename, &file_len);
			if (!file_data) {
				puts("[x] failed to read pak file data (does it exist in-pak?) :c");
				destroy_quit(1);
			}

			dump_file = fopen(cur->filename, "wb");
			if (fwrite(file_data, 1, file_len, dump_file) != file_len) {
				printf("[x] failed to write output %s :c", cur->filename);
				fclose(dump_file);
				free(file_data);
				continue;
			}
			fclose(dump_file);
			free(file_data);

			printf("[i] extracted %s\n", cur->filename);
		}
	}

	if (options.create || options.add || options.delete) write_posipak(pak, f[0]);
	if (pak) destroy_posipak(pak);
	return 0;
}