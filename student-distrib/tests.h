#ifndef TESTS_H
#define TESTS_H

// clears the screen
void clear_start();
// test launcher
void launch_tests();
// checks if we can divide by zero or not
int divzero_test(int a, int b);
// checks if non - memory allocated is present or not
char* not_present_check(char *a, char b);
// checks to see if we have an out of bounds allocated for memory or not for kernel
char* out_of_bounds_check_kernel(char *a, char b);
// checks to see if we have an out of bounds allocated for memory or not for video memory
char* out_of_bounds_check_video(char *a, char b);
// checks if memory allocated is present or not
void present_check();
// checks if we can dereference a null pointer
char* deref_null();
// type flag to switch between RTC and keyboard tests
int type_flag;

// prints frame0.txt
void frame0();
// prints frame1.txt
void frame1();
// prints parts of the very long text
void very_long();
// prints all of the very long text
void very_long_all();
// opens a non existent file
void open_not_exist();
// tries to close stdin and stdout
void close_stds();
// prints start and end of fish
void fish();
// prints all of fish
void fish_all();
// prints all of ls
void exec_ls_all();
// prints start and end of ls
void exec_ls();
// prints start and end of grep
void exec_grep();
// prints all of grep
void exec_grep_all();
// opens, reads, writes, and closes a file
void orwc_file();
// opens, reads, writes, and closes a directory
void orwc_dir();
// prints all of the files and stats in directory
void ls_dir();
// sees if you can read more than the file
void reading_more_than_file();
// sees if you can read past the end of the file
void reading_past_file_end();
// sees if you can read no bytes
void read_none();
// sees if you can do consecutive reads
void read_by_parts();
// checks if null is valid or not
void read_null();

#endif /* TESTS_H */
