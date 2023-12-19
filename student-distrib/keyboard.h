#ifndef _X_KEYBOARD_H
#define _X_KEYBOARD_H

#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "page.h"

#define caps_clicked 0x3A

#define left_shift_held 0x2A
#define left_shift_released 0xAA

#define right_shift_held 0x36
#define right_shift_released 0xB6

#define ctrl_held 0x1D
#define ctrl_released 0x9D

#define alt_held 0x38
#define alt_released 0xB8

#define F1 0x3B
#define F2 0x3C
#define F3 0x3D

int colorFlag;
int clearFlag;
int terminal;
int top_terminal_pid[4];

int32_t terminal_open(const uint8_t* filename);
int32_t terminal_close(int32_t fd);
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);
void setTerminal(int next_terminal);

//Create a key map for all the keys from 0 to 0x59 to print to screen
static const char keyMap[] = {
    '\0', // Undefined
    '\0', // Escape
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    '0',
    '-',
    '=',
    '\b', // Backspace
    '\t', // Tab
    'q',
    'w',
    'e',
    'r',
    't',
    'y',
    'u',
    'i',
    'o',
    'p',
    '[',
    ']',
    '\n', // Enter
    '\0', // Left Control
    'a',
    's',
    'd',
    'f',
    'g',
    'h',
    'j',
    'k',
    'l',
    ';',
    '\'',
    '`',
    '\0', // Left Shift
    '\\',
    'z',
    'x',
    'c',
    'v',
    'b',
    'n',
    'm',
    ',',
    '.',
    '/',
    '\0', // Right Shift
    '*', // Keypad *
    '\0', // Left Alt
    ' ',
};

static const char keyMapShift[] = {
    '\0', // Undefined
    '\0', // Escape
    '!',
    '@',
    '#',
    '$',
    '%',
    '^',
    '&',
    '*',
    '(',
    ')',
    '_',
    '+',
    '\b', // Backspace
    '\t', // Tab
    'Q',
    'W',
    'E',
    'R',
    'T',
    'Y',
    'U',
    'I',
    'O',
    'P',
    '{',
    '}',
    '\n', // Enter
    '\0', // Left Control
    'A',
    'S',
    'D',
    'F',
    'G',
    'H',
    'J',
    'K',
    'L',
    ':',
    '"',
    '~',
    '\0', // Left Shift
    '|',
    'Z',
    'X',
    'C',
    'V',
    'B',
    'N',
    'M',
    '<',
    '>',
    '?',
    '\0', // Right Shift
    '*',
    '\0', // Left Alt
    ' '
};

static const char keyMapShift2[] = {
    '\0', // Undefined
    '\0', // Escape
    '!',
    '@',
    '#',
    '$',
    '%',
    '^',
    '&',
    '*',
    '(',
    ')',
    '_',
    '+',
    '\b', // Backspace
    '\t', // Tab
    'q',
    'w',
    'e',
    'r',
    't',
    'y',
    'u',
    'i',
    'o',
    'p',
    '{',
    '}',
    '\n', // Enter
    '\0', // Left Control
    'a',
    's',
    'd',
    'f',
    'g',
    'h',
    'j',
    'k',
    'l',
    ':',
    '"',
    '~',
    '\0', // Left Shift
    '|',
    'z',
    'x',
    'c',
    'v',
    'b',
    'n',
    'm',
    '<',
    '>',
    '?',
    '\0', // Right Shift
    '*',
    '\0', // Left Alt
    ' '
};

static const char keyMapCaps[] = {
    '\0', // Undefined
    '\0', // Escape
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    '0',
    '-',
    '=',
    '\b', // Backspace
    '\t', // Tab
    'Q',
    'W',
    'E',
    'R',
    'T',
    'Y',
    'U',
    'I',
    'O',
    'P',
    '[',
    ']',
    '\n', // Enter
    '\0', // Left Control
    'A',
    'S',
    'D',
    'F',
    'G',
    'H',
    'J',
    'K',
    'L',
    ';',
    '\'',
    '`',
    '\0', // Left Shift
    '\\',
    'Z',
    'X',
    'C',
    'V',
    'B',
    'N',
    'M',
    ',',
    '.',
    '/',
    '\0', // Right Shift
    '*', // Keypad *
    '\0', // Left Alt
    ' ',
};


void kb_init();

#endif 
