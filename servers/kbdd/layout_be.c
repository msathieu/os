#include <kbd/layout.h>
#include <keyboard.h>

const struct layout be_layout = {
  .country = "be",
  .keys[0] = {
    [KEY_ESCAPE] = {.type = KBD_SPECIAL, .value = KBD_ESCAPE},
    [KEY_F1] = {.type = KBD_SPECIAL, .value = KBD_F1},
    [KEY_F2] = {.type = KBD_SPECIAL, .value = KBD_F2},
    [KEY_F3] = {.type = KBD_SPECIAL, .value = KBD_F3},
    [KEY_F4] = {.type = KBD_SPECIAL, .value = KBD_F4},
    [KEY_F5] = {.type = KBD_SPECIAL, .value = KBD_F5},
    [KEY_F6] = {.type = KBD_SPECIAL, .value = KBD_F6},
    [KEY_F7] = {.type = KBD_SPECIAL, .value = KBD_F7},
    [KEY_F8] = {.type = KBD_SPECIAL, .value = KBD_F8},
    [KEY_F9] = {.type = KBD_SPECIAL, .value = KBD_F9},
    [KEY_F10] = {.type = KBD_SPECIAL, .value = KBD_F10},
    [KEY_F11] = {.type = KBD_SPECIAL, .value = KBD_F11},
    [KEY_F12] = {.type = KBD_SPECIAL, .value = KBD_F12},
    [KEY_BACKSPACE] = {.type = KBD_SPECIAL, .value = KBD_BACKSPACE},
    [KEY_TAB] = {.value = '\t'},
    [KEY_CAPS_LOCK] = {.type = KBD_SPECIAL, .value = KBD_CAPS_LOCK, .toggle = 1 << KBD_TOGGLE_CAPS_LOCK},
    [KEY_ENTER] = {.value = '\n'},
    [KEY_LSHIFT] = {.type = KBD_SPECIAL, .value = KBD_SHIFT, .modifier = 1 << KBD_MODIFIER_SHIFT},
    [KEY_RSHIFT] = {.type = KBD_SPECIAL, .value = KBD_SHIFT, .modifier = 1 << KBD_MODIFIER_SHIFT},
    [KEY_LCONTROL] = {.type = KBD_SPECIAL, .value = KBD_CONTROL},
    [KEY_LSUPER] = {.type = KBD_SPECIAL, .value = KBD_SUPER},
    [KEY_LALT] = {.type = KBD_SPECIAL, .value = KBD_ALT},
    [KEY_SPACE] = {.value = ' '},
    [KEY_RALT] = {.type = KBD_SPECIAL, .value = KBD_ALTGR, .modifier = 1 << KBD_MODIFIER_ALTGR},
    [KEY_RSUPER] = {.type = KBD_SPECIAL, .value = KBD_SUPER},
    [KEY_MENU] = {.type = KBD_SPECIAL, .value = KBD_MENU},
    [KEY_RCONTROL] = {.type = KBD_SPECIAL, .value = KBD_CONTROL},
    [KEY_PRINT_SCREEN] = {.type = KBD_SPECIAL, .value = KBD_PRINT_SCREEN},
    [KEY_SCROLL_LOCK] = {.type = KBD_SPECIAL, .value = KBD_SCROLL_LOCK},
    [KEY_INSERT] = {.type = KBD_SPECIAL, .value = KBD_INSERT},
    [KEY_HOME] = {.type = KBD_SPECIAL, .value = KBD_HOME},
    [KEY_PAGE_UP] = {.type = KBD_SPECIAL, .value = KBD_PAGE_UP},
    [KEY_DELETE] = {.type = KBD_SPECIAL, .value = KBD_DELETE},
    [KEY_END] = {.type = KBD_SPECIAL, .value = KBD_END},
    [KEY_PAGE_DOWN] = {.type = KBD_SPECIAL, .value = KBD_PAGE_DOWN},
    [KEY_ARROW_UP] = {.type = KBD_SPECIAL, .value = KBD_ARROW_UP},
    [KEY_ARROW_LEFT] = {.type = KBD_SPECIAL, .value = KBD_ARROW_LEFT},
    [KEY_ARROW_DOWN] = {.type = KBD_SPECIAL, .value = KBD_ARROW_DOWN},
    [KEY_ARROW_RIGHT] = {.type = KBD_SPECIAL, .value = KBD_ARROW_RIGHT},
    [KEY_NUM_LOCK] = {.type = KBD_SPECIAL, .value = KBD_NUM_LOCK, .toggle = 1 << KBD_TOGGLE_NUM_LOCK},
    [KEY_NUM_SLASH] = {.value = '/'},
    [KEY_NUM_ASTERISK] = {.value = '*'},
    [KEY_NUM_DASH] = {.value = '-'},
    [KEY_NUM_PLUS] = {.value = '+'},
    [KEY_NUM_ENTER] = {.value = '\n'}},
  .keys[1] = {
    [KEY_1] = {.value = '&'},
    [KEY_3] = {.value = '"'},
    [KEY_4] = {.value = '\''},
    [KEY_5] = {.value = '('},
    [KEY_8] = {.value = '!'},
    [KEY_DASH] = {.value = ')'},
    [KEY_EQUALS] = {.value = '-'},
    [KEY_Q] = {.value = 'a'},
    [KEY_W] = {.value = 'z'},
    [KEY_E] = {.value = 'e'},
    [KEY_R] = {.value = 'r'},
    [KEY_T] = {.value = 't'},
    [KEY_Y] = {.value = 'y'},
    [KEY_U] = {.value = 'u'},
    [KEY_I] = {.value = 'i'},
    [KEY_O] = {.value = 'o'},
    [KEY_P] = {.value = 'p'},
    [KEY_BRACKET_OPENING] = {.value = '^'},
    [KEY_BRACKET_CLOSING] = {.value = '$'},
    [KEY_A] = {.value = 'q'},
    [KEY_S] = {.value = 's'},
    [KEY_D] = {.value = 'd'},
    [KEY_F] = {.value = 'f'},
    [KEY_G] = {.value = 'g'},
    [KEY_H] = {.value = 'h'},
    [KEY_J] = {.value = 'j'},
    [KEY_K] = {.value = 'k'},
    [KEY_L] = {.value = 'l'},
    [KEY_SEMICOLON] = {.value = 'm'},
    [KEY_INTERNATIONAL] = {.value = '<'},
    [KEY_Z] = {.value = 'w'},
    [KEY_X] = {.value = 'x'},
    [KEY_C] = {.value = 'c'},
    [KEY_V] = {.value = 'v'},
    [KEY_B] = {.value = 'b'},
    [KEY_N] = {.value = 'n'},
    [KEY_M] = {.value = ','},
    [KEY_COMMA] = {.value = ';'},
    [KEY_PERIOD] = {.value = ':'},
    [KEY_SLASH] = {.value = '='},
  },
  .keys[2] = {
    [KEY_1] = {.value = '1'},
    [KEY_2] = {.value = '2'},
    [KEY_3] = {.value = '3'},
    [KEY_4] = {.value = '4'},
    [KEY_5] = {.value = '5'},
    [KEY_6] = {.value = '6'},
    [KEY_7] = {.value = '7'},
    [KEY_8] = {.value = '8'},
    [KEY_9] = {.value = '9'},
    [KEY_0] = {.value = '0'},
    [KEY_EQUALS] = {.value = '_'},
    [KEY_Q] = {.value = 'A'},
    [KEY_W] = {.value = 'Z'},
    [KEY_E] = {.value = 'E'},
    [KEY_R] = {.value = 'R'},
    [KEY_T] = {.value = 'T'},
    [KEY_Y] = {.value = 'Y'},
    [KEY_U] = {.value = 'U'},
    [KEY_I] = {.value = 'I'},
    [KEY_O] = {.value = 'O'},
    [KEY_P] = {.value = 'P'},
    [KEY_BRACKET_CLOSING] = {.value = '*'},
    [KEY_A] = {.value = 'Q'},
    [KEY_S] = {.value = 'S'},
    [KEY_D] = {.value = 'D'},
    [KEY_F] = {.value = 'F'},
    [KEY_G] = {.value = 'G'},
    [KEY_H] = {.value = 'H'},
    [KEY_J] = {.value = 'J'},
    [KEY_K] = {.value = 'K'},
    [KEY_L] = {.value = 'L'},
    [KEY_SEMICOLON] = {.value = 'M'},
    [KEY_QUOTE] = {.value = '%'},
    [KEY_INTERNATIONAL] = {.value = '>'},
    [KEY_Z] = {.value = 'W'},
    [KEY_X] = {.value = 'X'},
    [KEY_C] = {.value = 'C'},
    [KEY_V] = {.value = 'V'},
    [KEY_B] = {.value = 'B'},
    [KEY_N] = {.value = 'N'},
    [KEY_M] = {.value = '?'},
    [KEY_COMMA] = {.value = '.'},
    [KEY_PERIOD] = {.value = '/'},
    [KEY_SLASH] = {.value = '+'},
  },
  .keys[3] = {
    [KEY_NUM_7] = {.type = KBD_SPECIAL, .value = KBD_HOME},
    [KEY_NUM_8] = {.type = KBD_SPECIAL, .value = KBD_ARROW_UP},
    [KEY_NUM_9] = {.type = KBD_SPECIAL, .value = KBD_PAGE_UP},
    [KEY_NUM_4] = {.type = KBD_SPECIAL, .value = KBD_ARROW_LEFT},
    [KEY_NUM_6] = {.type = KBD_SPECIAL, .value = KBD_ARROW_RIGHT},
    [KEY_NUM_1] = {.type = KBD_SPECIAL, .value = KBD_END},
    [KEY_NUM_2] = {.type = KBD_SPECIAL, .value = KBD_ARROW_DOWN},
    [KEY_NUM_3] = {.type = KBD_SPECIAL, .value = KBD_PAGE_DOWN},
    [KEY_NUM_0] = {.type = KBD_SPECIAL, .value = KBD_INSERT},
    [KEY_NUM_PERIOD] = {.type = KBD_SPECIAL, .value = KBD_DELETE},
  },
  .keys[4] = {
    [KEY_NUM_7] = {.value = '7'},
    [KEY_NUM_8] = {.value = '8'},
    [KEY_NUM_9] = {.value = '9'},
    [KEY_NUM_4] = {.value = '4'},
    [KEY_NUM_5] = {.value = '5'},
    [KEY_NUM_6] = {.value = '6'},
    [KEY_NUM_1] = {.value = '1'},
    [KEY_NUM_2] = {.value = '2'},
    [KEY_NUM_3] = {.value = '3'},
    [KEY_NUM_0] = {.value = '0'},
    [KEY_NUM_PERIOD] = {.value = '.'},
  },
  .keys[5] = {
    [KEY_1] = {.value = '|'},
    [KEY_2] = {.value = '@'},
    [KEY_3] = {.value = '#'},
    [KEY_6] = {.value = '^'},
    [KEY_9] = {.value = '{'},
    [KEY_0] = {.value = '}'},
    [KEY_BRACKET_OPENING] = {.value = '['},
    [KEY_BRACKET_CLOSING] = {.value = ']'},
    [KEY_BACKSLASH] = {.value = '`'},
    [KEY_INTERNATIONAL] = {.value = '\\'},
    [KEY_SLASH] = {.value = '~'},
  },
  .combinations[0] = {.parts = {1, 3}},
  .combinations[1 << KBD_MODIFIER_SHIFT] = {.parts = {2, 3}},
  .combinations[1 << KBD_TOGGLE_NUM_LOCK] = {.parts = {1, 4}},
  .combinations[1 << KBD_MODIFIER_SHIFT | 1 << KBD_TOGGLE_NUM_LOCK] = {.parts = {2, 4}},
  .combinations[1 << KBD_MODIFIER_ALTGR] = {.parts = {5, 1, 3}},
  .combinations[1 << KBD_MODIFIER_SHIFT | 1 << KBD_MODIFIER_ALTGR] = {.parts = {5, 2, 3}},
  .combinations[1 << KBD_TOGGLE_NUM_LOCK | 1 << KBD_MODIFIER_ALTGR] = {.parts = {5, 1, 4}},
  .combinations[1 << KBD_MODIFIER_SHIFT | 1 << KBD_TOGGLE_NUM_LOCK | 1 << KBD_MODIFIER_ALTGR] = {.parts = {5, 2, 4}}};
