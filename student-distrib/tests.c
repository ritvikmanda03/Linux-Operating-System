#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "page.h"
#include "file_sys.h"
#include "keyboard.h"
#include "rtc.h"


#define PASS 1
#define FAIL 0
#define bufSize 128

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $0x80");
}

/* 
 *   clear_start()
 *   DESCRIPTION: clears the screen and shifts to the top
 *   INPUTS: none
 *   OUTPUTS: none
 *   SIDE EFFECTS: The screen will clear and the next print will occur at the top
 */
void clear_start(){
    clear();
	int i;
	for(i = 0; i < 3; i++){
		printf("\n");
	}
	printf(" ");
}


/*************************************** Checkpoint 1 tests ***************************************/

/* 
 *   divzero_test()
 *   DESCRIPTION: Divides by zero to check division error exception
 *   INPUTS: integer a, integer b
 *   OUTPUTS: result of division
 *   SIDE EFFECTS: The image will fail and throw a division error exception
 */
int divzero_test(int a, int b){
	return a / b;
}

/* 
 *   not_present_check()
 *   DESCRIPTION: Dereferences a memory location that should not hold memory (should not be present)
 *   INPUTS: character pointer a, character pointer b
 *   OUTPUTS: result of dereference
 *   SIDE EFFECTS: The image will fail and throw a page fault exception
 */
char* not_present_check(char *a, char b){
		a = (char*)0;
		b = *a;
		return a;
}

/* 
 *   deref_null()
 *   DESCRIPTION: Dereferences a memory location that does not exist (is NULL)
 *   INPUTS: NONE
 *   OUTPUTS: result of dereference
 *   SIDE EFFECTS: The image will fail and throw a page fault exception
 */
char* deref_null(){
	char * a = NULL;
	char b;
	b = *(a);
	return a;
}

/* 
 *   out_of_bounds_check_kernel()
 *   DESCRIPTION: Dereferences a location past the assigned memory (past 8MB of memory)
 *   INPUTS: character pointer a, character pointer b
 *   OUTPUTS: result of dereference
 *   SIDE EFFECTS: The image will fail and throw a page fault exception
 */
char* out_of_bounds_check_kernel(char *a, char b){
		a = (char*)0x800000; // this memory address should not exist
		b = *a;
		return a;
}

/* 
 *   out_of_bounds_check_video()
 *   DESCRIPTION: Dereferences a location past the assigned memory (past video memory)
 *   INPUTS: character pointer a, character pointer b
 *   OUTPUTS: result of dereference
 *   SIDE EFFECTS: The image will fail and throw a page fault exception
 */
char* out_of_bounds_check_video(char *a, char b){
		a = (char*)0xB9000; // this memory address should not exist
		b = *a;
		return a;
}

/* 
 *   present_check()
 *   DESCRIPTION: Checks all memory locations that should be present. This includes all 4MB of
 * 	 			  memory that consists of the kernel page, and the 4KB page that consists of 
 * 				  video memory.
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   SIDE EFFECTS: The image should pass. If the kernel or video memory is allocated incorrectly this will cause
 * 				   a page fault.
 */
void present_check(){
	//kernel memory access check (every location from 0x400000 to 0x7FFFFF)
		int i;
		for(i = 0; i < 0x400000; i++){				//iterating to next mem address (next 4 bytes) is page fault
			unsigned char * pointer = (unsigned char*)(i + 0x400000);
			unsigned char temp;
			temp = *(pointer);
		}

		//video memory access check (every location from 0xB8000 to 0xB8FFF)
		for(i = 0; i < 4096; i++){				//iterating to next mem address (next 4 bytes) is page fault
			unsigned char * pointer = (unsigned char*)(i + 0xB8000);
			unsigned char temp;
			temp = *(pointer);
		}
		
} 


/* 
 *   struct_test()
 *   DESCRIPTION: Checks to see whether or not the memory allocated in the struct is valid or not.
 * 				  If it is not valid, then the function should give us a page fault. As a result, this
 *                function is made so there should not be a page fault.
 *   INPUTS: none
 *   OUTPUTS: none
 *   SIDE EFFECTS: If a memory allocated space renders invalid when it should be valid or vice versa,
 *				   then, the function issues a page fault exception. 
 */
void struct_test(){
	char * a = NULL;
	char b;
	//checks to see if first 4 MB is valid
	if(pd[0].p != 1){
		b = *(a);
		
	}	
	// checks to see if kernel 4MB is valid
	if(pd[1].p != 1){
		b = *(a);
	
	}
	// checks to see if 8MB+ is invalid or not
	if(pd[2].p == 1){ 
		b = *(a);
	
	}
	// checks to see if 8MB+ is invalid or not
	if(pt[1].p == 1){ 
		b = *(a);
	}
	// checks to see if table entry 1 is invalid
	if(pt[0].p == 1){ 
		b = *(a);
	}
}

/*
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

/*************************************** Checkpoint 2 tests ***************************************/


/* 
 *   frame0()
 *   DESCRIPTION: opens frame0.txt, finds the length of the file, reads the file using a buffer, 
 *   prints the loaded buffer, and closes the file
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   SIDE EFFECTS: prints the contents of frame0.txt to the console
 */
void frame0(){
	/* clears and initializes file descriptor and return value */
	clear();
	int fd, rv;

	/* opens the file and finds the length of the file */
    fd = open_file((const uint8_t*)"frame0.txt");
	if(fd == -1){TEST_OUTPUT("frame0", FAIL);}
	int len = inodes[file_array[fd].inode_idx].length;
	
	/* creates the buffer of the length of the file */
    char c1[len+1];

    c1[len] = '\0';
	/* reads the file and populates the buffer */
    rv = read_file(fd, c1, len);
	if(rv == -1){TEST_OUTPUT("frame0", FAIL);}
	/* prints the buffer onto the console */
	printf("%s", c1);
	/* closes the file after everything */
	int closed = close_file(fd);
	if(closed == -1){TEST_OUTPUT("frame0", FAIL);}

	TEST_OUTPUT("frame0", PASS);
}

/* 
 *   frame1()
 *   DESCRIPTION: opens frame1.txt, finds the length of the file, reads the file using a buffer, 
 *   prints the loaded buffer, and closes the file
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   SIDE EFFECTS: prints the contents of frame1.txt to the console
 */
void frame1(){
	/* clears and initializes file descriptor and return value */
	clear();
	int fd, rv;

	/* opens the file and finds the length of the file */
    fd = open_file((const uint8_t*)"frame1.txt");
	if(fd == -1){TEST_OUTPUT("frame1", FAIL);}
	int len = inodes[file_array[fd].inode_idx].length;
	
	/* creates the buffer of the length of the file */
    char c1[len+1];

    c1[len] = '\0';
	/* reads the file and populates the buffer */
    rv = read_file(fd, c1, len);
	if(rv == -1){TEST_OUTPUT("frame1", FAIL);}
	/* prints the buffer onto the console */
	printf("%s", c1);
	/* closes the file after everything */
	int closed = close_file(fd);
	if(closed == -1){TEST_OUTPUT("frame1", FAIL);}

	TEST_OUTPUT("frame1", PASS);
}

/* 
 *   very_long()
 *   DESCRIPTION: opens verylargetextwithverylongname.txt, finds the length of the file, reads the start and end of the file using a buffer, 
 *   prints the loaded buffer, and closes the file
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   SIDE EFFECTS: prints the start and end content of verylargetextwithverylongname.txt to the console
 */
void very_long(){
	/* clears and initializes file descriptor and return value */
	clear();
	int fd, rv;

	/* opens the file and finds the length of the file */
    fd = open_file((const uint8_t*)"verylargetextwithverylongname.tx");
	if(fd == -1){TEST_OUTPUT("very_long", FAIL);}
	int len = inodes[file_array[fd].inode_idx].length;
	
	/* creates the buffer of the length of the file */
    char c1[len+1];
    
	
    c1[len] = '\0';
	/* reads the file and populates the buffer */
    rv = read_file(fd, c1, len);
	if(rv == -1){TEST_OUTPUT("very_long", FAIL);}
	// prints the first 20 characters of the file
	int i;	
	for(i = 0; i < 20; i++){
		putc(c1[i]);
	}
	printf("\n");
	// prints the last 20 characters of the file
	for(i = sizeof(c1) - 20; i < sizeof(c1); i++){
		putc(c1[i]);
	}
	// closes the file after everything	
	int closed = close_file(fd);
	if(closed == -1){TEST_OUTPUT("very_long", FAIL);}

	TEST_OUTPUT("very_long", PASS);
}


/* 
 *   open_not_exist()
 *   DESCRIPTION: This function sees if you can open a non existent file
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   SIDE EFFECTS: NONE
 */void open_not_exist(){
	clear();

	/* clears and initializes file descriptor and return value */	
	int fd;

	/* opens the file through the file descriptor */    
	fd = open_file((const uint8_t*)"nonexistent.txt");
	if(fd != -1){TEST_OUTPUT("open_not_exist", FAIL);}

	/* sees if the open_file actually opened the non existent file or not */	
	TEST_OUTPUT("open_not_exist", PASS);

}


/* 
 *   close_stds()
 *   DESCRIPTION: This function sees if you can close stdin or stdout
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   SIDE EFFECTS: NONE
 */void close_stds(){
	clear();

	/* clears */	
	int closed = close_file(0);
	// tries to close the stdin in the file_array
	if(closed != -1){TEST_OUTPUT("close_stdin", FAIL);}
	closed = close_file(1);

	// tries to close the stdout in the file_array
	if(closed != -1){TEST_OUTPUT("close_stdout", FAIL);}
	TEST_OUTPUT("close_stds", PASS);

	// you should not be able to close the stdin or stdout
 }

/* 
 *   very_long()
 *   DESCRIPTION: opens verylargetextwithverylongname.txt, finds the length of the file, reads the file using a buffer, 
 *   prints the loaded buffer, and closes the file
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   SIDE EFFECTS: prints the contents of verylargetextwithverylongname.txt to the console
 */
void very_long_all(){
	clear();
	int fd, rv;

	/* clears and initializes file descriptor and return value */
    fd = open_file((const uint8_t*)"verylargetextwithverylongname.tx");
	if(fd == -1){TEST_OUTPUT("very_long_all", FAIL);}

	/* opens the file and finds the length of the file */	int len = inodes[file_array[fd].inode_idx].length;
	
    char c1[len+1];

	/* creates the buffer of the length of the file */    
    c1[len] = '\0';
    rv = read_file(fd, c1, len);
	if(rv == -1){TEST_OUTPUT("very_long_all", FAIL);}
	/* prints the buffer to the console */
    printf("%s", c1);
	/* reads the file and populates the buffer */
	int closed = close_file(fd);

	if(closed == -1){TEST_OUTPUT("very_long_all", FAIL);}


	/* closes the file after everything */	TEST_OUTPUT("very_long_all", PASS);
}

/* 
 *   fish()
 *   DESCRIPTION: opens fish, finds the length of the file, reads the start and end of the file using a buffer, 
 *   prints the loaded buffer, and closes the file
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   SIDE EFFECTS: prints the contents of fish to the console
 */	
void fish(){
	clear();

	int fd, rv;


	// clears and initializes the file descriptor and return value    
	fd = open_file((const uint8_t*)"fish");
	if(fd == -1){TEST_OUTPUT("fish", FAIL);}
	int len = inodes[file_array[fd].inode_idx].length;

	// opens the file	
    char c1[len+1];


	// finds the length of the file and creates buffer of the size    
	c1[len] = '\0';
	// reads the file and populates buffer	
    rv = read_file(fd, c1, len);
	if(rv == -1){TEST_OUTPUT("fish", FAIL);}

	int i;	

	// prints the first 35 characters of the file
	for(i = 0; i < 35; i++){
		putc(c1[i]);
	}
	printf("\n");
	for(i = sizeof(c1) - 20; i < sizeof(c1); i++){
		putc(c1[i]);
	}


	// prints the last 20 characters of the file	
	int closed = close_file(fd);
	if(closed == -1){TEST_OUTPUT("fish", FAIL);}


	// closes the file	
	TEST_OUTPUT("fish", PASS);
}

/* 
 *   frame1()
 *   DESCRIPTION: opens frame1.txt, finds the length of the file, reads the file using a buffer, 
 *   prints the loaded buffer, and closes the file
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   SIDE EFFECTS: prints the contents of frame1.txt to the console
 */
void fish_all(){
	// clears and initializes the file descriptor and return value
	clear();
	int fd, rv;

	// open the file	
    fd = open_file((const uint8_t*)"fish");
	if(fd == -1){TEST_OUTPUT("fish_all", FAIL);}
	int len = inodes[file_array[fd].inode_idx].length;

    char c1[len+1];
    

	// find the length of the file and initialize the buffer    
	c1[len] = '\0';
	// read and populate the buffer
    rv = read_file(fd, c1, len);
	if(rv == -1){TEST_OUTPUT("fish_all", FAIL);}
    int i;
	for(i = 0; i < sizeof(c1); i++){
		putc(c1[i]);
	}
	
	// print the buffer to the console	
	int closed = close_file(fd);
	if(closed == -1){TEST_OUTPUT("fish_all", FAIL);}
	
	// close the file after we are done	
	TEST_OUTPUT("fish_all", PASS);
}

/* 
 *   exec_ls_all()
 *   DESCRIPTION: opens ls, finds the length of the file, reads the file using a buffer, 
 *   prints the loaded buffer, and closes the file
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   SIDE EFFECTS: prints the contents of ls executable to the console
 */
void exec_ls_all(){
	clear();
	int fd, rv;
	//opens the ls file
    fd = open_file((const uint8_t*)"ls");
	if(fd == -1){TEST_OUTPUT("exec_ls_all", FAIL);}
	//finds the length of the ls file
	int len = inodes[file_array[fd].inode_idx].length;
	
    char c1[len+1];

    c1[len] = '\0';
	//attempts to read all of the ls file
    rv = read_file(fd, c1, len);
	if(rv == -1){TEST_OUTPUT("exec_ls_all", FAIL);}
	int i;
	for(i = 0; i < sizeof(c1); i++){
		putc(c1[i]);
	}
	//close ls file and make sure it succeeded
	int closed = close_file(fd);
	if(closed == -1){TEST_OUTPUT("exec_ls_all", FAIL);}

	TEST_OUTPUT("exec_ls_all", PASS);
}

/* 
 *   exec_ls()
 *   DESCRIPTION: opens ls, finds the length of the file, reads the beginnign and end of the file using a buffer, 
 *   prints the loaded buffer, and closes the file
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   SIDE EFFECTS: prints the contents of the beginnign and end of ls executable to the console
 */
void exec_ls(){
	clear();
	int fd, rv;
	//open the ls file
    fd = open_file((const uint8_t*)"ls");
	if(fd == -1){TEST_OUTPUT("exec_ls", FAIL);}
	//find the length of the ls file
	int len = inodes[file_array[fd].inode_idx].length;
	
    char c1[len+1];

    c1[len] = '\0';
	//read the entire ls file
    rv = read_file(fd, c1, len);
	if(rv == -1){TEST_OUTPUT("exec_ls", FAIL);}
	//print the beginnign and the end of the file
	int i;	
	for(i = 0; i < 10; i++){
		putc(c1[i]);
	}
	printf("\n");
	for(i = sizeof(c1) - 38; i < sizeof(c1); i++){
		putc(c1[i]);
	}
	//close ls file and make sure it succeeded
	int closed = close_file(fd);
	if(closed == -1){TEST_OUTPUT("exec_ls", FAIL);}

	TEST_OUTPUT("exec_ls", PASS);
}

/* 
 *   exec_grep_all()
 *   DESCRIPTION: opens grep, finds the length of the file, reads the file using a buffer, 
 *   prints the loaded buffer, and closes the file
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   SIDE EFFECTS: prints the contents of grep executable to the console
 */
void exec_grep_all(){
	clear();
	int fd, rv;
	//open the grep file
    fd = open_file((const uint8_t*)"grep");
	if(fd == -1){TEST_OUTPUT("exec_grep_all", FAIL);}
	//find the length of the grep file
	int len = inodes[file_array[fd].inode_idx].length;
	
    char c1[len+1];

    c1[len] = '\0';
	//read the entire grep file
    rv = read_file(fd, c1, len);
	if(rv == -1){TEST_OUTPUT("exec_grep_all", FAIL);}
	int i;
	//print the entire grep file
	for(i = 0; i < sizeof(c1); i++){
		putc(c1[i]);
	}
	//close grep file and make sure it succeeded
	int closed = close_file(fd);
	if(closed == -1){TEST_OUTPUT("exec_grep_all", FAIL);}

	TEST_OUTPUT("exec_grep_all", PASS);
}

/* 
 *   exec_grep()
 *   DESCRIPTION: opens grep, finds the length of the file, reads the beginnign and end of the file using a buffer, 
 *   prints the loaded buffer, and closes the file
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   SIDE EFFECTS: prints the contents of the beginnign and end of the grep executable to the console
 */
void exec_grep(){
	clear();
	int fd, rv;

	//open the grep file
    fd = open_file((const uint8_t*)"grep");
	if(fd == -1){TEST_OUTPUT("exec_grep", FAIL);}
	//find the length of the grep file
	int len = inodes[file_array[fd].inode_idx].length;
	
    char c1[len+1];

    c1[len] = '\0';
	//read teh entire grep file
    rv = read_file(fd, c1, len);
	if(rv == -1){TEST_OUTPUT("exec_grep", FAIL);}
	//print only the beginning and end of the grep file
	int i;	
	for(i = 0; i < 10; i++){
		putc(c1[i]);
	}
	printf("\n");
	for(i = sizeof(c1) - 38; i < sizeof(c1); i++){
		putc(c1[i]);
	}
	int closed = close_file(fd);
	if(closed == -1){TEST_OUTPUT("exec_grep", FAIL);}

	TEST_OUTPUT("exec_grep", PASS);
}

/* 
 *   orwc_file()
 *   DESCRIPTION: opens frame0.txt, attempts to read from frame0.txt, attempts to write to frame0.txt,
 * 				  and closes frame0.txt.
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   SIDE EFFECTS: prints a pass or fail statement depending on alignment to expected response.
 */
void orwc_file(){
	clear();
	int fd, rv;

	TEST_HEADER;
	//open the frame0 file
    fd = open_file((const uint8_t*)"frame0.txt");
	if(fd == -1){TEST_OUTPUT("orwc", FAIL);}
	int len = inodes[file_array[fd].inode_idx].length;
	char c1[len+1];

    c1[len] = '\0';
	//read from the frame0 file
    rv = read_file(fd, c1, len);
	if(rv == -1){TEST_OUTPUT("orwc", FAIL);}
	//try writing to the frame0 file
	int rv2 = write_file(fd, c1, 1);
	if(rv2 != -1){TEST_OUTPUT("orwc", FAIL);}
	//close the frame0 file
	int closed = close_file(fd);
	if(closed == -1){TEST_OUTPUT("orwc", FAIL);}

	TEST_OUTPUT("orwc", PASS);
}

/* 
 *   orwc_dir()
 *   DESCRIPTION: opens "." (current directory), attempts to read from current directory, attempts to write to current directory,
 * 				  and closes "." (current directory).
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   SIDE EFFECTS: prints a pass or fail statement depending on alignment to expected response.
 */
void orwc_dir(){
	clear();
	int fd, rv;

	TEST_HEADER;
	//open the current directory
    fd = open_dir((const uint8_t*)".");
	if(fd == -1){TEST_OUTPUT("orwc_dir", FAIL);}
	int len = inodes[file_array[fd].inode_idx].length;
	char c1[len+1];

    c1[len] = '\0';
	//read a filename from the current directory
    rv = read_dir(fd, c1, len);
	if(rv == -1){TEST_OUTPUT("orwc_dir", FAIL);}
	//write to the current directory
	int rv2 = write_dir(fd, c1, 1);
	if(rv2 != -1){TEST_OUTPUT("orwc_dir", FAIL);}

	int i;
	for(i = 0; i < 32; i++){
		if(c1[i] != '\0'){
			putc(c1[i]);
		}
		else{ break; }
	}
	//close the current directory
	int closed = close_dir(fd);
	if(closed == -1){TEST_OUTPUT("orwc_dir", FAIL);}

	TEST_OUTPUT("orwc_di", PASS);
}

/* 
 *   ls_dir()
 *   DESCRIPTION: opens the current directory, reads all the filenames from the directory in order,
 * 				  writes the filenames, file sizes, and file types in organized fashion, and closes the directory.
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   SIDE EFFECTS: prints out a result that looks like the "ls -l" command with all filenames and info
 */
void ls_dir(){
	clear();
	int fd;

	//TEST_HEADER;

	//open the current directory
	fd = open_dir((const uint8_t*)".");
	if(fd == -1){TEST_OUTPUT("ls_dir", FAIL);}    
	//find the length of a dir
	int len = inodes[file_array[fd].inode_idx].length;
	char c1[len+1];

    c1[len] = '\0';

	//print all the filenames in the directory in a sorted manner
	while(read_dir(fd, c1, len) != 0){
		printf("file_name: ");
		//align the filename such that it has enough spaces to align on the right side
		int i;
		for(i = 0; i < 32; i++){
			if(c1[i] == '\0'){
				putc(' ');
			}
		}
		for(i = 0; i < 32; i++){
			if(c1[i] != '\0'){
				putc(c1[i]);
			}
		}

		//find and print all the file information
		dentry_t temp;
		read_dentry_by_name((const uint8_t*)c1, &temp);
		printf(", file_type: ");
		int ft = (int)temp.file_type;
		printf("%d", ft);
		printf(", file_size: ");
		int len = (int)inodes[temp.inode_n].length;
		int len_copy = len;
		int digits = 0;

		//find num digits to align size to the right side
		do {
			len_copy /= 10;
			++digits;
		} while (len_copy != 0);

		for(i = 0; i < 6-digits; i++){
			putc(' ');
		}

		printf("%d", len);
		printf("\n");
	}
	//close the directory
	int closed = close_dir(fd);
	if(closed == -1){TEST_OUTPUT("ls_dir", FAIL);}

}

/* 
 *   reading_more_than_file()
 *   DESCRIPTION: opens the frame0.txt file, reads the entire file and past its end,
 * 				  wthen prints the buffer and closes the file.
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   SIDE EFFECTS: prints frame0 and a pass or fail statement depending on alignment to expected response.
 */
void reading_more_than_file(){
	clear();
	int fd, rv;
	//open frame0.txt
    fd = open_file((const uint8_t*)"frame0.txt");
	if(fd == -1){TEST_OUTPUT("reading_more_than_file", FAIL);}
	int len = inodes[file_array[fd].inode_idx].length;
	
    char c1[len+1];

    c1[len] = '\0';
	//try to read more than the actual size of the file
    rv = read_file(fd, c1, len+20);		//try to read 20 more chars than end of file
	if(rv == -1){TEST_OUTPUT("reading_more_than_file", FAIL);}
	printf("%s", c1);
	//close the file
	int closed = close_file(fd);
	if(closed == -1){TEST_OUTPUT("reading_more_than_file", FAIL);}

	TEST_OUTPUT("reading_more_than_file", PASS);
}

/* 
 *   reading_past_file_end()
 *   DESCRIPTION: opens the frame0.txt file, reads the entire file, then attempts reading past its end,
 * 				  then prints the buffer and closes the file.
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   SIDE EFFECTS: prints frame0 and a pass or fail statement depending on alignment to expected response.
 */
void reading_past_file_end(){
	clear();
	int fd, rv;

	//open frame0.txt
    fd = open_file((const uint8_t*)"frame0.txt");
	if(fd == -1){TEST_OUTPUT("reading_past_file_end", FAIL);}
	int len = inodes[file_array[fd].inode_idx].length;
	
    char c1[len+1];

    c1[len] = '\0';
	//first read the entire file
    rv = read_file(fd, c1, len);
	//then try to read bytes past the end of the file
	rv = read_file(fd, c1, 20);		//try to read 20 chars after we have reached the end of the file
	if(rv == -1){TEST_OUTPUT("reading_past_file_end", FAIL);}
	printf("%s", c1);
	//close the file
	int closed = close_file(fd);
	if(closed == -1){TEST_OUTPUT("reading_past_file_end", FAIL);}

	TEST_OUTPUT("reading_past_file_end", PASS);
}

/* 
 *   read_none()
 *   DESCRIPTION: opens frame0.txt, finds the length of the file, read 0 bytes of the file
 *   prints the loaded buffer, and closes the file
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   SIDE EFFECTS: prints a success message to console
 */
void read_none(){
	clear();
	int fd, rv;
	//open the frame0.txt file
    fd = open_file((const uint8_t*)"frame0.txt");	//open the frame0.txt file
	if(fd == -1){TEST_OUTPUT("read_none", FAIL);}
	
    char c1[1];

	//try to read 0 bytes of the file
    c1[0] = '\0';
    rv = read_file(fd, c1, 0);						//read 0 bytes to the buffer
	if(rv == -1){TEST_OUTPUT("read_none", FAIL);}
	printf("%s", c1);
	int closed = close_file(fd);					//close the file
	if(closed == -1){TEST_OUTPUT("read_none", FAIL);}
	TEST_OUTPUT("read_none", PASS);
}

/* 
 *   read_by_parts()
 *   DESCRIPTION: opens frame0.txt, finds the length of the file, reads it one byte at a time into buffer
 *   prints the loaded buffer, and closes the file
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   SIDE EFFECTS: prints the contents of frame0.txt to the console
 */
void read_by_parts(){
	clear();
	int fd, rv;

    fd = open_file((const uint8_t*)"frame0.txt");		//open frame0.txt file
	if(fd == -1){TEST_OUTPUT("read_by_parts", FAIL);}	//if it did not open, return -1
	int len = inodes[file_array[fd].inode_idx].length;	//get length of file from inode

	int i;
	for(i = 0; i < len; i++){
		char c1[2];			
    	c1[1] = '\0';									//create a buffer and initialize null char
    	rv = read_file(fd, c1, 1);						//read a single byte
		if(rv == -1){TEST_OUTPUT("read_by_parts", FAIL);}
		printf("%s", c1);								//print overall buffer
	}

	int closed = close_file(fd);						//close the file
	if(closed == -1){TEST_OUTPUT("read_by_parts", FAIL);}	

	TEST_OUTPUT("read_by_parts", PASS);
}

/* 
 *   read_null()
 *   DESCRIPTION: Sees if you can run a function if you pass in a null argument
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   SIDE EFFECTS: prints a success message to console
 */
void read_null(){
	clear();
	/* checks if arguments are null for all th functions */
	dentry_t * temp = NULL;
	int rv = read_dentry_by_name((const uint8_t*)"frame0.txt", temp);
	if(rv != -1){TEST_OUTPUT("read_null", FAIL);}
	
	dentry_t temp1;
	rv = read_dentry_by_name((const uint8_t*)NULL, &temp1);
	if(rv != -1){TEST_OUTPUT("read_null", FAIL);}
	
	dentry_t * temp2 = NULL;
	rv = read_dentry_by_index(4, temp2);
	if(rv != -1){TEST_OUTPUT("read_null", FAIL);}

	TEST_OUTPUT("read_null", PASS);
}




// content by index .. 

// add more tests here

/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	//Set high to show Keyboard test and set low to show RTC Test
	clear();
	type_flag = 1;

	//Test for IDT
	//TEST_OUTPUT("idt_test", idt_test());
	//Test for exceptions (Divide-by-0 and pagefault)
	// divzero_test(4, 0);

	//Testing system call for now
	// assertion_failure();

	//PAGE TEST
	// not_present_check(0,0);
	// deref_null();
	// present_check();
	// out_of_bounds_check_kernel(0,0);
	// out_of_bounds_check_video(0,0);
	// struct_test();

	int flaggy = 4;


	//TERMINAL DRIVER TEST
	if(flaggy == 1){
		int terminal;
		while(1){
			char buffer_read[bufSize]; 
			terminal = terminal_read(0, buffer_read, bufSize);
			terminal_write(0, buffer_read, terminal);
		}
	}
	// RTC WRITE/READ
	if(flaggy == 0){
		rtc_open(0);
		int a;
		int b;
		for(a = 2; a <= 1024; a*=2){
			rtc_write(0,&a,0);
			printf("Test for %d : \n",a);
			for(b = 0; b < a; b++){
				if(b < 78) printf("1");
				rtc_read(0,0,0);
			}
			printf("\n");
		} 
	}
	//RTC OPEN/READ
	if(flaggy == 2){
		int a = 128; //initialize to 128 hz for test purpose
		int b;
			rtc_write(0,&a,0);
			printf("Temporarily set to 128hz: ");
			for(b = 0; b < 50; b++){
				printf("1");
				rtc_read(0,0,0);
			}
			printf("\n");
			rtc_open(0);
			printf("Open changes freq to 2hz: ");
			for(b = 0; b < 50; b++){
				printf("1");
				rtc_read(0,0,0);
			} 
	}


	// FILE SYSTEM TESTS
	// frame0();
	// frame1();
	// very_long();
	// very_long_all();
	// open_not_exist();
	// close_stds();
	// fish();
	// fish_all();
	// exec_ls_all();
	// exec_ls();
	// exec_grep_all();
	// exec_grep();
	// orwc_file();
	// orwc_dir();
	// ls_dir();
	// reading_more_than_file();
	// reading_past_file_end();
	// read_none();
	// read_by_parts();
	// read_null();
}
