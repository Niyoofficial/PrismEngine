#pragma once

#define SCANCODE_MASK (1<<30)
#define SCANCODE_TO_KEYCODE(X)  ((int)(X) | SCANCODE_MASK)

namespace Prism
{

enum class ScanCode
{
    Unknown = 0,

    /**
     *  \name Usage page 0x07
     *
     *  These values are from usage page 0x07 (USB keyboard page).
     */
    /* @{ */

    A = 4,
    B = 5,
    C = 6,
    D = 7,
    E = 8,
    F = 9,
    G = 10,
    H = 11,
    I = 12,
    J = 13,
    K = 14,
    L = 15,
    M = 16,
    N = 17,
    O = 18,
    P = 19,
    Q = 20,
    R = 21,
    S = 22,
    T = 23,
    U = 24,
    V = 25,
    W = 26,
    X = 27,
    Y = 28,
    Z = 29,

    One = 30,
    Two = 31,
    Three = 32,
    Four = 33,
    Five = 34,
    Six = 35,
    Seven = 36,
    Eight = 37,
    Nine = 38,
    Zero = 39,

    Return = 40,
    Escape = 41,
    Backspace = 42,
    Tab = 43,
    Space = 44,

    Minus = 45,
    Equals = 46,
    Leftbracket = 47,
    Rightbracket = 48,
    Backslash = 49, /**< Located at the lower left of the return
                                  *   key on ISO keyboards and at the right end
                                  *   of the QWERTY row on ANSI keyboards.
                                  *   Produces REVERSE SOLIDUS (backslash) and
                                  *   VERTICAL LINE in a US layout, REVERSE
                                  *   SOLIDUS and VERTICAL LINE in a UK Mac
                                  *   layout, NUMBER SIGN and TILDE in a UK
                                  *   Windows layout, DOLLAR SIGN and POUND SIGN
                                  *   in a Swiss German layout, NUMBER SIGN and
                                  *   APOSTROPHE in a German layout, GRAVE
                                  *   ACCENT and POUND SIGN in a French Mac
                                  *   layout, and ASTERISK and MICRO SIGN in a
                                  *   French Windows layout.
                                  */
    Nonushash = 50, /**< ISO USB keyboards actually use this code
                                  *   instead of 49 for the same key, but all
                                  *   OSes I've seen treat the two codes
                                  *   identically. So, as an implementor, unless
                                  *   your keyboard generates both of those
                                  *   codes and your OS treats them differently,
                                  *   you should generate BACKSLASH
                                  *   instead of this code. As a user, you
                                  *   should not rely on this code because SDL
                                  *   will never generate it with most (all?)
                                  *   keyboards.
                                  */
    Semicolon = 51,
    Apostrophe = 52,
    Grave = 53, /**< Located in the top left corner (on both ANSI
                              *   and ISO keyboards). Produces GRAVE ACCENT and
                              *   TILDE in a US Windows layout and in US and UK
                              *   Mac layouts on ANSI keyboards, GRAVE ACCENT
                              *   and NOT SIGN in a UK Windows layout, SECTION
                              *   SIGN and PLUS-MINUS SIGN in US and UK Mac
                              *   layouts on ISO keyboards, SECTION SIGN and
                              *   DEGREE SIGN in a Swiss German layout (Mac:
                              *   only on ISO keyboards), CIRCUMFLEX ACCENT and
                              *   DEGREE SIGN in a German layout (Mac: only on
                              *   ISO keyboards), SUPERSCRIPT TWO and TILDE in a
                              *   French Windows layout, COMMERCIAL AT and
                              *   NUMBER SIGN in a French Mac layout on ISO
                              *   keyboards, and LESS-THAN SIGN and GREATER-THAN
                              *   SIGN in a Swiss German, German, or French Mac
                              *   layout on ANSI keyboards.
                              */
    Comma = 54,
    Period = 55,
    Slash = 56,

    CapsLock = 57,

    F1 = 58,
    F2 = 59,
    F3 = 60,
    F4 = 61,
    F5 = 62,
    F6 = 63,
    F7 = 64,
    F8 = 65,
    F9 = 66,
    F10 = 67,
    F11 = 68,
    F12 = 69,

    Printscreen = 70,
    Scrolllock = 71,
    Pause = 72,
    Insert = 73, /**< insert on PC, help on some Mac keyboards (but
                                   does send code 73, not 117) */
    Home = 74,
    Pageup = 75,
    Delete = 76,
    End = 77,
    Pagedown = 78,
    Right = 79,
    Left = 80,
    Down = 81,
    Up = 82,

    Numlockclear = 83, /**< num lock on PC, clear on Mac keyboards
                                     */
    Kp_divide = 84,
    Kp_multiply = 85,
    Kp_minus = 86,
    Kp_plus = 87,
    Kp_enter = 88,
    Kp_1 = 89,
    Kp_2 = 90,
    Kp_3 = 91,
    Kp_4 = 92,
    Kp_5 = 93,
    Kp_6 = 94,
    Kp_7 = 95,
    Kp_8 = 96,
    Kp_9 = 97,
    Kp_0 = 98,
    Kp_period = 99,

    NonusbackslaSH = 100, /**< This is the additional key that ISO
                                        *   keyboards have over ANSI ones,
                                        *   located between left shift and Y.
                                        *   Produces GRAVE ACCENT and TILDE in a
                                        *   US or UK Mac layout, REVERSE SOLIDUS
                                        *   (backslash) and VERTICAL LINE in a
                                        *   US or UK Windows layout, and
                                        *   LESS-THAN SIGN and GREATER-THAN SIGN
                                        *   in a Swiss German, German, or French
                                        *   layout. */
    Application = 101, /**< windows contextual menu, compose */
    Power = 102, /**< The USB document says this is a status flag,
                               *   not a physical key - but some Mac keyboards
                               *   do have a power key. */
    Kp_equals = 103,
    F13 = 104,
    F14 = 105,
    F15 = 106,
    F16 = 107,
    F17 = 108,
    F18 = 109,
    F19 = 110,
    F20 = 111,
    F21 = 112,
    F22 = 113,
    F23 = 114,
    F24 = 115,
    Execute = 116,
    Help = 117,    /**< AL Integrated Help Center */
    Menu = 118,    /**< Menu (show menu) */
    Select = 119,
    Stop = 120,    /**< AC Stop */
    Again = 121,   /**< AC Redo/Repeat */
    Undo = 122,    /**< AC Undo */
    Cut = 123,     /**< AC Cut */
    Copy = 124,    /**< AC Copy */
    Paste = 125,   /**< AC Paste */
    Find = 126,    /**< AC Find */
    Mute = 127,
    Volumeup = 128,
    Volumedown = 129,
/* not sure whether there's a reason to enable these */
/*     LOCKINGCAPSLOCK = 130,  */
/*     LOCKINGNUMLOCK = 131, */
/*     LOCKINGSCROLLLOCK = 132, */
    Kp_comma = 133,
    Kp_equalsas400 = 134,

    International1 = 135, /**< used on Asian keyboards, see
                                            footnotes in USB doc */
    International2 = 136,
    International3 = 137, /**< Yen */
    International4 = 138,
    International5 = 139,
    International6 = 140,
    International7 = 141,
    International8 = 142,
    International9 = 143,
    Lang1 = 144, /**< Hangul/English toggle */
    Lang2 = 145, /**< Hanja conversion */
    Lang3 = 146, /**< Katakana */
    Lang4 = 147, /**< Hiragana */
    Lang5 = 148, /**< Zenkaku/Hankaku */
    Lang6 = 149, /**< reserved */
    Lang7 = 150, /**< reserved */
    Lang8 = 151, /**< reserved */
    Lang9 = 152, /**< reserved */

    AltErase = 153,    /**< Erase-Eaze */
    Sysreq = 154,
    Cancel = 155,      /**< AC Cancel */
    Clear = 156,
    Prior = 157,
    Return2 = 158,
    Separator = 159,
    Out = 160,
    Oper = 161,
    Clearagain = 162,
    Crsel = 163,
    Exsel = 164,

    Kp_00 = 176,
    Kp_000 = 177,
    Thousandsseparator = 178,
    Decimalseparator = 179,
    Currencyunit = 180,
    Currencysubunit = 181,
    Kp_leftparen = 182,
    Kp_rightparen = 183,
    Kp_leftbrace = 184,
    Kp_rightbrace = 185,
    Kp_tab = 186,
    Kp_backspace = 187,
    Kp_a = 188,
    Kp_b = 189,
    Kp_c = 190,
    Kp_d = 191,
    Kp_e = 192,
    Kp_f = 193,
    Kp_xor = 194,
    Kp_power = 195,
    Kp_percent = 196,
    Kp_less = 197,
    Kp_greater = 198,
    Kp_ampersand = 199,
    Kp_dblampersand = 200,
    Kp_verticalbar = 201,
    Kp_dblverticalbar = 202,
    Kp_colon = 203,
    Kp_hash = 204,
    Kp_space = 205,
    Kp_at = 206,
    Kp_exclam = 207,
    Kp_memstore = 208,
    Kp_memrecall = 209,
    Kp_memclear = 210,
    Kp_memadd = 211,
    Kp_memsubtract = 212,
    Kp_memmultiply = 213,
    Kp_memdivide = 214,
    Kp_plusminus = 215,
    Kp_clear = 216,
    Kp_clearentry = 217,
    Kp_binary = 218,
    Kp_octal = 219,
    Kp_decimal = 220,
    Kp_hexadecimal = 221,

    Lctrl = 224,
    Lshift = 225,
    Lalt = 226, /**< alt, option */
    Lgui = 227, /**< windows, command (apple), meta */
    Rctrl = 228,
    Rshift = 229,
    Ralt = 230, /**< alt gr, option */
    Rgui = 231, /**< windows, command (apple), meta */

    Mode = 257,    /**< I'm not sure if this is really not covered
                                 *   by any of the above, but since there's a
                                 *   special SDL_KMOD_MODE for it I'm adding it here
                                 */

    /* @} *//* Usage page 0x07 */

    /**
     *  \name Usage page 0x0C
     *
     *  These values are mapped from usage page 0x0C (USB consumer page).
     *  See https://usb.org/sites/default/files/hut1_2.pdf
     *
     *  There are way more keys in the spec than we can represent in the
     *  current scancode range, so pick the ones that commonly come up in
     *  real world usage.
     */
    /* @{ */

    Audionext = 258,
    Audioprev = 259,
    Audiostop = 260,
    Audioplay = 261,
    Audiomute = 262,
    Mediaselect = 263,
    Www = 264,             /**< AL Internet Browser */
    Mail = 265,
    Calculator = 266,      /**< AL Calculator */
    Computer = 267,
    Ac_search = 268,       /**< AC Search */
    Ac_home = 269,         /**< AC Home */
    Ac_back = 270,         /**< AC Back */
    Ac_forward = 271,      /**< AC Forward */
    Ac_stop = 272,         /**< AC Stop */
    Ac_refresh = 273,      /**< AC Refresh */
    Ac_bookmarks = 274,    /**< AC Bookmarks */

    /* @} *//* Usage page 0x0C */

    /**
     *  \name Walther keys
     *
     *  These are values that Christian Walther added (for mac keyboard?).
     */
    /* @{ */

    Brightnessdown = 275,
    Brightnessup = 276,
    Displayswitch = 277, /**< display mirroring/dual display
                                           switch, video mode switch */
    Kbdillumtoggle = 278,
    Kbdillumdown = 279,
    Kbdillumup = 280,
    Eject = 281,
    Sleep = 282,           /**< SC System Sleep */

    App1 = 283,
    App2 = 284,

    /* @} *//* Walther keys */

    /**
     *  \name Usage page 0x0C (additional media keys)
     *
     *  These values are mapped from usage page 0x0C (USB consumer page).
     */
    /* @{ */

    Audiorewind = 285,
    Audiofastforward = 286,

    /* @} *//* Usage page 0x0C (additional media keys) */

    /**
     *  \name Mobile keys
     *
     *  These are values that are often used on mobile phones.
     */
    /* @{ */

    SoftLeft = 287, /**< Usually situated below the display on phones and
                                      used as a multi-function feature key for selecting
                                      a software defined function shown on the bottom left
                                      of the display. */
    SoftRight = 288, /**< Usually situated below the display on phones and
                                       used as a multi-function feature key for selecting
                                       a software defined function shown on the bottom right
                                       of the display. */
    Call = 289, /**< Used for accepting phone calls. */
    EndCall = 290, /**< Used for rejecting phone calls. */

    /* @} *//* Mobile keys */

    /* Add any other keys here. */

    NumScancodes = 512 /**< not a key, just marks the number of scancodes
                                 for array bounds */
};

enum class KeyCode
{
    Unknown = 0,

    Return = '\r',
    Escape = '\x1B',
    Backspace = '\b',
    Tab = '\t',
    Space = ' ',
    Exclaim = '!',
    Quotedbl = '"',
    Hash = '#',
    Percent = '%',
    Dollar = '$',
    Ampersand = '&',
    Quote = '\'',
    Leftparen = '(',
    Rightparen = ')',
    Asterisk = '*',
    Plus = '+',
    Comma = ',',
    Minus = '-',
    Period = '.',
    Slash = '/',
    Zero = '0',
    One = '1',
    Two = '2',
    Three = '3',
    Four = '4',
    Five = '5',
    Six = '6',
    Seven = '7',
    Eight = '8',
    Nine = '9',
    Colon = ':',
    Semicolon = ';',
    Less = '<',
    Equals = '=',
    Greater = '>',
    Question = '?',
    At = '@',

    /*
       Skip uppercase letters
     */

    Leftbracket = '[',
    Backslash = '\\',
    Rightbracket = ']',
    Caret = '^',
    Underscore = '_',
    Backquote = '`',
    A = 'a',
    B = 'b',
    C = 'c',
    D = 'd',
    E = 'e',
    F = 'f',
    G = 'g',
    H = 'h',
    I = 'i',
    J = 'j',
    K = 'k',
    L = 'l',
    M = 'm',
    N = 'n',
    O = 'o',
    P = 'p',
    Q = 'q',
    R = 'r',
    S = 's',
    T = 't',
    U = 'u',
    V = 'v',
    W = 'w',
    X = 'x',
    Y = 'y',
    Z = 'z',

    Capslock = SCANCODE_TO_KEYCODE(ScanCode::CapsLock),

    F1 = SCANCODE_TO_KEYCODE(ScanCode::F1),
    F2 = SCANCODE_TO_KEYCODE(ScanCode::F2),
    F3 = SCANCODE_TO_KEYCODE(ScanCode::F3),
    F4 = SCANCODE_TO_KEYCODE(ScanCode::F4),
    F5 = SCANCODE_TO_KEYCODE(ScanCode::F5),
    F6 = SCANCODE_TO_KEYCODE(ScanCode::F6),
    F7 = SCANCODE_TO_KEYCODE(ScanCode::F7),
    F8 = SCANCODE_TO_KEYCODE(ScanCode::F8),
    F9 = SCANCODE_TO_KEYCODE(ScanCode::F9),
    F10 = SCANCODE_TO_KEYCODE(ScanCode::F10),
    F11 = SCANCODE_TO_KEYCODE(ScanCode::F11),
    F12 = SCANCODE_TO_KEYCODE(ScanCode::F12),

    Printscreen = SCANCODE_TO_KEYCODE(ScanCode::Printscreen),
    Scrolllock = SCANCODE_TO_KEYCODE(ScanCode::Scrolllock),
    Pause = SCANCODE_TO_KEYCODE(ScanCode::Pause),
    Insert = SCANCODE_TO_KEYCODE(ScanCode::Insert),
    Home = SCANCODE_TO_KEYCODE(ScanCode::Home),
    Pageup = SCANCODE_TO_KEYCODE(ScanCode::Pageup),
    Delete = '\x7F',
    End = SCANCODE_TO_KEYCODE(ScanCode::End),
    Pagedown = SCANCODE_TO_KEYCODE(ScanCode::Pagedown),
    Right = SCANCODE_TO_KEYCODE(ScanCode::Right),
    Left = SCANCODE_TO_KEYCODE(ScanCode::Left),
    Down = SCANCODE_TO_KEYCODE(ScanCode::Down),
    Up = SCANCODE_TO_KEYCODE(ScanCode::Up),

    Numlockclear = SCANCODE_TO_KEYCODE(ScanCode::Numlockclear),
    Kp_divide = SCANCODE_TO_KEYCODE(ScanCode::Kp_divide),
    Kp_multiply = SCANCODE_TO_KEYCODE(ScanCode::Kp_multiply),
    Kp_minus = SCANCODE_TO_KEYCODE(ScanCode::Kp_minus),
    Kp_plus = SCANCODE_TO_KEYCODE(ScanCode::Kp_plus),
    Kp_enter = SCANCODE_TO_KEYCODE(ScanCode::Kp_enter),
    Kp_1 = SCANCODE_TO_KEYCODE(ScanCode::Kp_1),
    Kp_2 = SCANCODE_TO_KEYCODE(ScanCode::Kp_2),
    Kp_3 = SCANCODE_TO_KEYCODE(ScanCode::Kp_3),
    Kp_4 = SCANCODE_TO_KEYCODE(ScanCode::Kp_4),
    Kp_5 = SCANCODE_TO_KEYCODE(ScanCode::Kp_5),
    Kp_6 = SCANCODE_TO_KEYCODE(ScanCode::Kp_6),
    Kp_7 = SCANCODE_TO_KEYCODE(ScanCode::Kp_7),
    Kp_8 = SCANCODE_TO_KEYCODE(ScanCode::Kp_8),
    Kp_9 = SCANCODE_TO_KEYCODE(ScanCode::Kp_9),
    Kp_0 = SCANCODE_TO_KEYCODE(ScanCode::Kp_0),
    Kp_period = SCANCODE_TO_KEYCODE(ScanCode::Kp_period),

    Application = SCANCODE_TO_KEYCODE(ScanCode::Application),
    Power = SCANCODE_TO_KEYCODE(ScanCode::Power),
    Kp_equals = SCANCODE_TO_KEYCODE(ScanCode::Kp_equals),
    F13 = SCANCODE_TO_KEYCODE(ScanCode::F13),
    F14 = SCANCODE_TO_KEYCODE(ScanCode::F14),
    F15 = SCANCODE_TO_KEYCODE(ScanCode::F15),
    F16 = SCANCODE_TO_KEYCODE(ScanCode::F16),
    F17 = SCANCODE_TO_KEYCODE(ScanCode::F17),
    F18 = SCANCODE_TO_KEYCODE(ScanCode::F18),
    F19 = SCANCODE_TO_KEYCODE(ScanCode::F19),
    F20 = SCANCODE_TO_KEYCODE(ScanCode::F20),
    F21 = SCANCODE_TO_KEYCODE(ScanCode::F21),
    F22 = SCANCODE_TO_KEYCODE(ScanCode::F22),
    F23 = SCANCODE_TO_KEYCODE(ScanCode::F23),
    F24 = SCANCODE_TO_KEYCODE(ScanCode::F24),
    Execute = SCANCODE_TO_KEYCODE(ScanCode::Execute),
    Help = SCANCODE_TO_KEYCODE(ScanCode::Help),
    Menu = SCANCODE_TO_KEYCODE(ScanCode::Menu),
    Select = SCANCODE_TO_KEYCODE(ScanCode::Select),
    Stop = SCANCODE_TO_KEYCODE(ScanCode::Stop),
    Again = SCANCODE_TO_KEYCODE(ScanCode::Again),
    Undo = SCANCODE_TO_KEYCODE(ScanCode::Undo),
    Cut = SCANCODE_TO_KEYCODE(ScanCode::Cut),
    Copy = SCANCODE_TO_KEYCODE(ScanCode::Copy),
    Paste = SCANCODE_TO_KEYCODE(ScanCode::Paste),
    Find = SCANCODE_TO_KEYCODE(ScanCode::Find),
    Mute = SCANCODE_TO_KEYCODE(ScanCode::Mute),
    Volumeup = SCANCODE_TO_KEYCODE(ScanCode::Volumeup),
    Volumedown = SCANCODE_TO_KEYCODE(ScanCode::Volumedown),
    Kp_comma = SCANCODE_TO_KEYCODE(ScanCode::Kp_comma),
    Kp_equalsas400 = SCANCODE_TO_KEYCODE(ScanCode::Kp_equalsas400),

    Alterase = SCANCODE_TO_KEYCODE(ScanCode::AltErase),
    Sysreq = SCANCODE_TO_KEYCODE(ScanCode::Sysreq),
    Cancel = SCANCODE_TO_KEYCODE(ScanCode::Cancel),
    Clear = SCANCODE_TO_KEYCODE(ScanCode::Clear),
    Prior = SCANCODE_TO_KEYCODE(ScanCode::Prior),
    Return2 = SCANCODE_TO_KEYCODE(ScanCode::Return2),
    Separator = SCANCODE_TO_KEYCODE(ScanCode::Separator),
    Out = SCANCODE_TO_KEYCODE(ScanCode::Out),
    Oper = SCANCODE_TO_KEYCODE(ScanCode::Oper),
    Clearagain = SCANCODE_TO_KEYCODE(ScanCode::Clearagain),
    Crsel = SCANCODE_TO_KEYCODE(ScanCode::Crsel),
    Exsel = SCANCODE_TO_KEYCODE(ScanCode::Exsel),

    Kp_00 = SCANCODE_TO_KEYCODE(ScanCode::Kp_00),
    Kp_000 = SCANCODE_TO_KEYCODE(ScanCode::Kp_000),
    Thousandsseparator = SCANCODE_TO_KEYCODE(ScanCode::Thousandsseparator),
    Decimalseparator = SCANCODE_TO_KEYCODE(ScanCode::Decimalseparator),
    Currencyunit = SCANCODE_TO_KEYCODE(ScanCode::Currencyunit),
    Currencysubunit = SCANCODE_TO_KEYCODE(ScanCode::Currencysubunit),
    Kp_leftparen = SCANCODE_TO_KEYCODE(ScanCode::Kp_leftparen),
    Kp_rightparen = SCANCODE_TO_KEYCODE(ScanCode::Kp_rightparen),
    Kp_leftbrace = SCANCODE_TO_KEYCODE(ScanCode::Kp_leftbrace),
    Kp_rightbrace = SCANCODE_TO_KEYCODE(ScanCode::Kp_rightbrace),
    Kp_tab = SCANCODE_TO_KEYCODE(ScanCode::Kp_tab),
    Kp_backspace = SCANCODE_TO_KEYCODE(ScanCode::Kp_backspace),
    Kp_a = SCANCODE_TO_KEYCODE(ScanCode::Kp_a),
    Kp_b = SCANCODE_TO_KEYCODE(ScanCode::Kp_b),
    Kp_c = SCANCODE_TO_KEYCODE(ScanCode::Kp_c),
    Kp_d = SCANCODE_TO_KEYCODE(ScanCode::Kp_d),
    Kp_e = SCANCODE_TO_KEYCODE(ScanCode::Kp_e),
    Kp_f = SCANCODE_TO_KEYCODE(ScanCode::Kp_f),
    Kp_xor = SCANCODE_TO_KEYCODE(ScanCode::Kp_xor),
    Kp_power = SCANCODE_TO_KEYCODE(ScanCode::Kp_power),
    Kp_percent = SCANCODE_TO_KEYCODE(ScanCode::Kp_percent),
    Kp_less = SCANCODE_TO_KEYCODE(ScanCode::Kp_less),
    Kp_greater = SCANCODE_TO_KEYCODE(ScanCode::Kp_greater),
    Kp_ampersand = SCANCODE_TO_KEYCODE(ScanCode::Kp_ampersand),
    Kp_dblampersand = SCANCODE_TO_KEYCODE(ScanCode::Kp_dblampersand),
    Kp_verticalbar = SCANCODE_TO_KEYCODE(ScanCode::Kp_verticalbar),
    Kp_dblverticalbar = SCANCODE_TO_KEYCODE(ScanCode::Kp_dblverticalbar),
    Kp_colon = SCANCODE_TO_KEYCODE(ScanCode::Kp_colon),
    Kp_hash = SCANCODE_TO_KEYCODE(ScanCode::Kp_hash),
    Kp_space = SCANCODE_TO_KEYCODE(ScanCode::Kp_space),
    Kp_at = SCANCODE_TO_KEYCODE(ScanCode::Kp_at),
    Kp_exclam = SCANCODE_TO_KEYCODE(ScanCode::Kp_exclam),
    Kp_memstore = SCANCODE_TO_KEYCODE(ScanCode::Kp_memstore),
    Kp_memrecall = SCANCODE_TO_KEYCODE(ScanCode::Kp_memrecall),
    Kp_memclear = SCANCODE_TO_KEYCODE(ScanCode::Kp_memclear),
    Kp_memadd = SCANCODE_TO_KEYCODE(ScanCode::Kp_memadd),
    Kp_memsubtract = SCANCODE_TO_KEYCODE(ScanCode::Kp_memsubtract),
    Kp_memmultiply = SCANCODE_TO_KEYCODE(ScanCode::Kp_memmultiply),
    Kp_memdivide = SCANCODE_TO_KEYCODE(ScanCode::Kp_memdivide),
    Kp_plusminus = SCANCODE_TO_KEYCODE(ScanCode::Kp_plusminus),
    Kp_clear = SCANCODE_TO_KEYCODE(ScanCode::Kp_clear),
    Kp_clearentry = SCANCODE_TO_KEYCODE(ScanCode::Kp_clearentry),
    Kp_binary = SCANCODE_TO_KEYCODE(ScanCode::Kp_binary),
    Kp_octal = SCANCODE_TO_KEYCODE(ScanCode::Kp_octal),
    Kp_decimal = SCANCODE_TO_KEYCODE(ScanCode::Kp_decimal),
    Kp_hexadecimal = SCANCODE_TO_KEYCODE(ScanCode::Kp_hexadecimal),

    Lctrl = SCANCODE_TO_KEYCODE(ScanCode::Lctrl),
    Lshift = SCANCODE_TO_KEYCODE(ScanCode::Lshift),
    Lalt = SCANCODE_TO_KEYCODE(ScanCode::Lalt),
    Lgui = SCANCODE_TO_KEYCODE(ScanCode::Lgui),
    Rctrl = SCANCODE_TO_KEYCODE(ScanCode::Rctrl),
    Rshift = SCANCODE_TO_KEYCODE(ScanCode::Rshift),
    Ralt = SCANCODE_TO_KEYCODE(ScanCode::Ralt),
    Rgui = SCANCODE_TO_KEYCODE(ScanCode::Rgui),

    Mode = SCANCODE_TO_KEYCODE(ScanCode::Mode),

    Audionext = SCANCODE_TO_KEYCODE(ScanCode::Audionext),
    Audioprev = SCANCODE_TO_KEYCODE(ScanCode::Audioprev),
    Audiostop = SCANCODE_TO_KEYCODE(ScanCode::Audiostop),
    Audioplay = SCANCODE_TO_KEYCODE(ScanCode::Audioplay),
    Audiomute = SCANCODE_TO_KEYCODE(ScanCode::Audiomute),
    Mediaselect = SCANCODE_TO_KEYCODE(ScanCode::Mediaselect),
    Www = SCANCODE_TO_KEYCODE(ScanCode::Www),
    Mail = SCANCODE_TO_KEYCODE(ScanCode::Mail),
    Calculator = SCANCODE_TO_KEYCODE(ScanCode::Calculator),
    Computer = SCANCODE_TO_KEYCODE(ScanCode::Computer),
    Ac_search = SCANCODE_TO_KEYCODE(ScanCode::Ac_search),
    Ac_home = SCANCODE_TO_KEYCODE(ScanCode::Ac_home),
    Ac_back = SCANCODE_TO_KEYCODE(ScanCode::Ac_back),
    Ac_forward = SCANCODE_TO_KEYCODE(ScanCode::Ac_forward),
    Ac_stop = SCANCODE_TO_KEYCODE(ScanCode::Ac_stop),
    Ac_refresh = SCANCODE_TO_KEYCODE(ScanCode::Ac_refresh),
    Ac_bookmarks = SCANCODE_TO_KEYCODE(ScanCode::Ac_bookmarks),

    Brightnessdown = SCANCODE_TO_KEYCODE(ScanCode::Brightnessdown),
    Brightnessup = SCANCODE_TO_KEYCODE(ScanCode::Brightnessup),
    Displayswitch = SCANCODE_TO_KEYCODE(ScanCode::Displayswitch),
    Kbdillumtoggle = SCANCODE_TO_KEYCODE(ScanCode::Kbdillumtoggle),
    Kbdillumdown = SCANCODE_TO_KEYCODE(ScanCode::Kbdillumdown),
    Kbdillumup = SCANCODE_TO_KEYCODE(ScanCode::Kbdillumup),
    Eject = SCANCODE_TO_KEYCODE(ScanCode::Eject),
    Sleep = SCANCODE_TO_KEYCODE(ScanCode::Sleep),
    App1 = SCANCODE_TO_KEYCODE(ScanCode::App1),
    App2 = SCANCODE_TO_KEYCODE(ScanCode::App2),

    Audiorewind = SCANCODE_TO_KEYCODE(ScanCode::Audiorewind),
    Audiofastforward = SCANCODE_TO_KEYCODE(ScanCode::Audiofastforward),

    SoftLeft = SCANCODE_TO_KEYCODE(ScanCode::SoftLeft),
    SoftRight = SCANCODE_TO_KEYCODE(ScanCode::SoftRight),
    Call = SCANCODE_TO_KEYCODE(ScanCode::Call),
    EndCall = SCANCODE_TO_KEYCODE(ScanCode::EndCall),

    LeftMouseButton,
    MiddleMouseButton,
    RightMouseButton,
    MouseButton4,
    MouseButton5
};
}
