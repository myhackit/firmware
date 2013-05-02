#include <sd-reader/fat.h>
#include <sd-reader/fat_config.h>
#include <sd-reader/partition.h>
#include <sd-reader/sd_raw.h>
#include <sd-reader/sd_raw_config.h>

uint8_t find_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name, struct fat_dir_entry_struct* dir_entry) {
    while(fat_read_dir(dd, dir_entry)) {
        if(strcmp(dir_entry->long_name, name) == 0) {
            fat_reset_dir(dd);
            return 1;
        }
    }
    return 0;
}

struct fat_file_struct* open_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name) {
    struct fat_dir_entry_struct file_entry;
    if(!find_file_in_dir(fs, dd, name, &file_entry)){
        return 0;
    }
    return fat_open_file(fs, &file_entry);
}

/*
char *rest, *token, *ptr;
char *rest2, *token2, *ptr2;
uint8_t which_map, map_col, map_row = 0;
void parse_layout(char* line){
	if(strcmp(line,"[MAIN]")==0){
		which_map = 0;
		map_row = 0;
	} else if(strcmp(line,"[TOGGLE]")==0){
		which_map = 1;
		map_row = 0;
	} else {
		map_col = 0;
		ptr = line;
		while((token = strtok_r(ptr, ",", &rest)) != NULL) {
			maps[which_map][map_row][map_col] = lookup_id(token);
			map_col++;
			ptr = rest;
		}
		map_row++;
	}
}
uint16_t lookup_id(const char* needle){
	for(int i=0;i<MAP_SIZE;i++){
		if(!strcmp(needle,str_map[i].name)){
			return str_map[i].id;
		}
	}
	return 0;
}
void prrint(char* line){
	for(uint8_t i=0;i<strlen(line);++i){
		usb_debug_putchar(line[i]);
	}
}
void lineize(struct fat_file_struct* fd, void (*process_line)(char*) ){
	uint8_t read_buffer[32];
	char *line = (char *)malloc(150);
	uint8_t pos = 0;

	while(fat_read_file(fd, read_buffer, sizeof(read_buffer)) > 0){
		for(uint8_t i=0;i<sizeof(read_buffer);++i){
			switch (read_buffer[i]) {
				case '\r': case '\t': case ' ': continue; // Ignore white space
				case '\n': //TODO: How to detect lines w/o newline before EOF?
					if(pos == 0) continue; // Ignore blank lines
					line[pos] = '\0';
					process_line(line);
					pos = 0;
				continue;
			}
			line[pos++] = read_buffer[i];
		}
	}
	free(line);
}
*/
/*
	// setup sd card slot
#if DEBUG
	if(!sd_raw_init()) {
		print("MMC/SD initialization failed\n");
	}
#endif

	// open first partition 
	struct partition_struct* partition = partition_open(sd_raw_read, sd_raw_read_interval, sd_raw_write, sd_raw_write_interval, 0);
	// If the partition did not open, assume the storage device
	// is a "superfloppy", i.e. has no MBR.
	if(!partition) {
		partition = partition_open(sd_raw_read, sd_raw_read_interval, sd_raw_write, sd_raw_write_interval, -1);
#if DEBUG
		if(!partition) {
			print("opening partition failed\n");
		}
#endif
	}

	// open file system
	struct fat_fs_struct* fs = fat_open(partition);
#if DEBUG
	if(!fs){
		print("opening filesystem failed\n");
	}
#endif

	// open root directory
	struct fat_dir_entry_struct directory;
	fat_get_dir_entry_of_path(fs, "/layouts/", &directory);

	struct fat_dir_struct* dd = fat_open_dir(fs, &directory);
#if DEBUG
	if(!dd){
		print("opening root directory failed\n");
	}
#endif

	struct fat_file_struct* fd = open_file_in_dir(fs, dd, "myhackit");
#if DEBUG
	if(!fd){
		print("error opening file\n");
	}
#endif

	lineize(fd, parse_layout);
	fat_close_file(fd);

	fd = open_file_in_dir(fs, dd, "macros");
	lineize(fd, parse_macro);
	fat_close_file(fd);
	*/
