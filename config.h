#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

#define CHAKSU_NEXT_IMAGE KEY_SPACE
#define CHAKSU_PREV_IMAGE KEY_BACKSPACE
#define CHAKSU_BG_COLOR  (Color){0x28,0x28,0x28, 0xff} //RGBA
#define CHAKSU_MESSAGE_COLOR (Color){0xff,0xff,0xff,0xff} //RGBA
#define CHAKSU_MESSAGE_ERR_COLOR (Color){0xff,0x00,0x00,0xff} //RGBA
#define CHAKSU_MESSAGE_FONT_SIZE 20
#define CHAKSU_WINDOW_WIDTH 1000
#define CHAKSU_WINDOW_HEIGHT 700
#define CHAKSU_FRAMERATE 60
#define CHAKSU_SCALE_FACTOR 0.3f
#define CHAKSU_MIN_SCALE 0.1f

// #define CHAKSU_CUSTOM_FONT "abolute or relative path of ttf font" // ttf font file path

#endif // config_h_INCLUDED
