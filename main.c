#include <string.h>
#include <math.h>
#include <stdio.h>

#include <webp/decode.h>

#include "raylib.h"
#include "vector.h"

#include "./config.h"

#include "resources.c"

#define IMPLEMENT_CONFIG_PARSER
#include "config_parser.h"

#define IMPLEMENT_UTIL
#include "util.h"

#define UNUSED(x) (void)x
#define WINDOW_TITLE "Chaksu Image Viewer"
#define CONFIG_FILE_NAME "chaksu.conf"
#define OFFSET 50
#define update_message(message, fmt, ...) snprintf(message, sizeof(message), fmt, __VA_ARGS__)

// https://www.reddit.com/r/C_Programming/comments/1i40cus/comment/m7tryqu/?utm_source=share&utm_medium=web3x&utm_name=web3xcss&utm_term=1&utm_content=share_button

#define MAX_FILE_EXTENSION_LEN 16
const char valid_extensions[][MAX_FILE_EXTENSION_LEN] = 
{
    ".png", ".jpg", ".jpeg", ".gif", ".psd", ".tga", ".bmp", ".ppm",
     ".pic", ".hdr", ".pvr",  ".qoi", ".dds", ".pkm", ".ktx", ".astc",".webp"
};


const int total_extensions = sizeof(valid_extensions)/sizeof(valid_extensions[0]);

typedef struct 
{
    int         window_width;
    int         window_height;
    int         chaksu_framerate;
    int         chaksu_message_font_size;

    float       chaksu_scale_factor;
    float       chaksu_min_scale;

    Color       chaksu_bg_color;
    Color       chaksu_message_color;
    Color       chaksu_message_err_color;

    KeyboardKey chaksu_next_image;
    KeyboardKey chaksu_prev_image;
    KeyboardKey chaksu_rotate_ccw;
    KeyboardKey chaksu_rotate_cw;
    KeyboardKey chaksu_fit_screen;

    char*       font_path;
} chaksu_config;

typedef struct
{
    const char*  config_file;
    const char** other_arguments;
    bool   load_recursive; // to be implemented.
} chaksu_arguments;

KeyboardKey str_to_keyboard_key(const char* key)
{
    #define str_eql(s1,s2) strcmp(s1,s2) == 0
    if(str_eql(key,"A")) return KEY_A;
    if(str_eql(key,"B")) return KEY_B;
    if(str_eql(key,"C")) return KEY_C;
    if(str_eql(key,"D")) return KEY_D;
    if(str_eql(key,"E")) return KEY_E;
    if(str_eql(key,"F")) return KEY_F;
    if(str_eql(key,"G")) return KEY_G;
    if(str_eql(key,"H")) return KEY_H;
    if(str_eql(key,"I")) return KEY_I;
    if(str_eql(key,"J")) return KEY_J;
    if(str_eql(key,"K")) return KEY_K;
    if(str_eql(key,"L")) return KEY_L;
    if(str_eql(key,"M")) return KEY_M;
    if(str_eql(key,"N")) return KEY_N;
    if(str_eql(key,"O")) return KEY_O;
    if(str_eql(key,"P")) return KEY_P;
    if(str_eql(key,"Q")) return KEY_Q;
    if(str_eql(key,"R")) return KEY_R;
    if(str_eql(key,"S")) return KEY_S;
    if(str_eql(key,"T")) return KEY_T;
    if(str_eql(key,"U")) return KEY_U;
    if(str_eql(key,"V")) return KEY_V;
    if(str_eql(key,"W")) return KEY_W;
    if(str_eql(key,"X")) return KEY_X;
    if(str_eql(key,"Y")) return KEY_Y;
    if(str_eql(key,"Z")) return KEY_Z;
    if(str_eql(key,"0")) return KEY_ZERO;
    if(str_eql(key,"1")) return KEY_ONE;
    if(str_eql(key,"2")) return KEY_TWO;
    if(str_eql(key,"3")) return KEY_THREE;
    if(str_eql(key,"4")) return KEY_FOUR;
    if(str_eql(key,"5")) return KEY_FIVE;
    if(str_eql(key,"6")) return KEY_SIX;
    if(str_eql(key,"7")) return KEY_SEVEN;
    if(str_eql(key,"8")) return KEY_EIGHT;
    if(str_eql(key,"9")) return KEY_NINE;
    if(str_eql(key,"ESCAPE")) return KEY_ESCAPE;
    if(str_eql(key,"ENTER")) return KEY_ENTER;
    if(str_eql(key,"SPACE")) return KEY_SPACE;
    if(str_eql(key,"TAB")) return KEY_TAB;
    if(str_eql(key,"BACKSPACE")) return KEY_BACKSPACE;
    if(str_eql(key,"INSERT")) return KEY_INSERT;
    if(str_eql(key,"DELETE")) return KEY_DELETE;
    if(str_eql(key,"RIGHT")) return KEY_RIGHT;
    if(str_eql(key,"LEFT")) return KEY_LEFT;
    if(str_eql(key,"DOWN")) return KEY_DOWN;
    if(str_eql(key,"UP")) return KEY_UP;
    if(str_eql(key,"F1")) return KEY_F1;
    if(str_eql(key,"F2")) return KEY_F2;
    if(str_eql(key,"F3")) return KEY_F3;
    if(str_eql(key,"F4")) return KEY_F4;
    if(str_eql(key,"F5")) return KEY_F5;
    if(str_eql(key,"F6")) return KEY_F6;
    if(str_eql(key,"F7")) return KEY_F7;
    if(str_eql(key,"F8")) return KEY_F8;
    if(str_eql(key,"F9")) return KEY_F9;
    if(str_eql(key,"F10")) return KEY_F10;
    if(str_eql(key,"F11")) return KEY_F11;
    if(str_eql(key,"F12")) return KEY_F12;
    return KEY_NULL;
}

bool config_get_keyboard_key(Config *config,
                             const char *key,
                             KeyboardKey* keyboard_key)
{
    char* key_pressed = NULL;
    if(!config_get_string(config,key,&key_pressed)) return false;
    
    KeyboardKey res = str_to_keyboard_key(str_to_upper(key_pressed));

    if(res == KEY_NULL) return false; 

    *keyboard_key = res;

    return true;
}

bool config_get_color(Config *config,const char *key,Color* color)
{
    char* hexcolor = NULL;
    if(!config_get_string(config,key,&hexcolor)) return false;

    int len = strlen(hexcolor);
    if(len<6 || len>8) goto invalid_color;

    unsigned char rgba[] = {0,0,0,255};
    for(int i=0,j=0; i<=len-2; i+=2,j++)
    {
        int first_half   = hex_digit_to_int(hexcolor[i]);
        int second_half  = hex_digit_to_int(hexcolor[i+1]);

        if(first_half<0 || second_half<0) goto invalid_color;

        first_half <<= 4;

        int full_byte  = first_half | second_half;
        if(full_byte<0 || full_byte>255) goto invalid_color;
        rgba[j] = (unsigned char)full_byte; 
    }

    color->r = rgba[0];
    color->g = rgba[1];
    color->b = rgba[2];
    color->a = rgba[3];

    return true;

    invalid_color:
        fprintf(stderr,"Invalid hex color: %s\n",hexcolor);
        return false;
}

Vector2 update_pos(Texture2D texture, float *scale)
{
    const int screen_width   = GetScreenWidth();
    const int screen_height  = GetScreenHeight() - OFFSET;

    if(texture.width <= screen_width&&
        texture.height <= screen_height)
    {
        *scale = 1;
        return (Vector2){(screen_width - texture.width) / 2.0,
                         (screen_height-texture.height) / 2.0};

    }

    const float aspect_ratio = (float)texture.width/texture.height;

    int new_width  = screen_width;
    int new_height = new_width / aspect_ratio;

    if (new_height > screen_height)
    {
        new_height = screen_height;
        new_width  = screen_height * aspect_ratio;
    }

    *scale = (float)new_width / texture.width;

    return (Vector2){(screen_width - new_width) / 2.0,
                     (screen_height - new_height) / 2.0};
}

bool is_image(const char *path)
{
    if (!IsPathFile(path))
        return false;

    for (int i = 0; i < total_extensions; i++)
    {
        if (IsFileExtension(path, valid_extensions[i]))
            return true;
    }

    return false;
}



char **get_images_from_dir__helper(char ***result, const char *dir,bool recursive)
{
    UNUSED(recursive);
    if (IsPathFile(dir))
        return NULL;

    char ***tmp = result;
    char **images;

    if (!tmp)
    {
        images = Vector(*images);

        if (!images)
            return NULL;

        tmp = &images;
    }

    FilePathList files = LoadDirectoryFiles(dir);

    for (unsigned int i = 0; i < files.count; i++)
    {
        if (is_image(files.paths[i]))
            vector_append(*tmp, str_duplicate(files.paths[i]));
    }

    UnloadDirectoryFiles(files);

    return *tmp;
}

char **get_images_from_dir(const char *dir,bool recursive)
{
    char **images = get_images_from_dir__helper(NULL, dir,recursive);

    if (!images)
        return NULL;

    return images;
}

char** get_all_valid_images(const char **args,const int n,bool recursive)
{
    (void)recursive;
    if (n < 1 || !args)
        return NULL;

    char **images = Vector(*images);

    if (!images)
        return NULL;

    for (int i = 0; i < n; i++)
    {
        if (IsPathFile(args[i]))
        {
            if (is_image(args[i]))
                vector_append(images, str_duplicate(args[i]));
        }
        else
        {
            (void)get_images_from_dir__helper(&images, args[i],recursive);
        }
    }

    return images;
}

chaksu_arguments parse_argument(const char** passed_args, const int n)
{
    chaksu_arguments parsed_argument = {
        .config_file = NULL, 
        .load_recursive = false,
        .other_arguments = NULL
    };

    if (n <= 1 || !passed_args)
    {
        return parsed_argument; 
    }

    const char** temp = Vector(*temp); 
    for (int i = 0; i < n; i++)
    {
        if(strcmp("-c",passed_args[i])==0||
           strcmp("--config",passed_args[i])==0)
        {
            if(i+1<n && IsPathFile(passed_args[i+1]))
            {
                parsed_argument.config_file = passed_args[++i];
            }
        }
        else if(strcmp("-r",passed_args[i])==0)
        {
            parsed_argument.load_recursive = true;
        }
        else
        {
            vector_append(temp,passed_args[i]);
        }
    }
    if(vector_length(temp) > 0) parsed_argument.other_arguments = temp;
    return parsed_argument;
}

static Texture load__webp(const char *file){
    Texture texture = {0};

    int data_size = 0;
    int width = 0;
    int height = 0;
    unsigned char *file_data = LoadFileData(file, &data_size);

    if(data_size == 0) return texture;

    uint8_t  *pixels = WebPDecodeRGBA(file_data, data_size,&width, &height);

    if(!pixels || height == 0 || width == 0) return texture;

    Image img = {.data    = pixels,
                 .mipmaps = 1,
                 .width  = width,
                 .height = height,
                 .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
                };

    texture = LoadTextureFromImage(img); 
    UnloadImage(img);
    return texture; 

}

Texture chaksu_load_texture(const char *file)
{
    Texture texture = {0};
    if(!is_image(file)) return texture;

    if(IsFileExtension(file, ".webp"))
    {
        return load__webp(file);
    }

    return LoadTexture(file);
}

chaksu_config default_config = {
    .window_width             = CHAKSU_WINDOW_WIDTH,
    .window_height            = CHAKSU_WINDOW_HEIGHT,
    .chaksu_framerate         = CHAKSU_FRAMERATE,
    .chaksu_message_font_size = CHAKSU_MESSAGE_FONT_SIZE,
    .chaksu_scale_factor      = CHAKSU_SCALE_FACTOR,
    .chaksu_min_scale         = CHAKSU_MIN_SCALE,
    .chaksu_bg_color          = CHAKSU_BG_COLOR,
    .chaksu_message_color     = CHAKSU_MESSAGE_COLOR,
    .chaksu_message_err_color = CHAKSU_MESSAGE_ERR_COLOR,
    .chaksu_next_image        = CHAKSU_NEXT_IMAGE,
    .chaksu_prev_image        = CHAKSU_PREV_IMAGE,
    .chaksu_rotate_ccw        = CHAKSU_ROTATE_CCW,
    .chaksu_rotate_cw         = CHAKSU_ROTATE_CW,
    .chaksu_fit_screen        = CHAKSU_FIT_SCREEN,
    .font_path                = NULL 
};

// char* get_config_file()
// {
//     FILE *f = fopen(CONFIG_FILE_NAME,"r");
//     return NULL;
// }

Config* chaksu_load_config(const char* cofig_file, chaksu_config* cfg)
{
    #define with_default(type,key,var,default_val)                                                         \
    do{                                                                                                    \
        if(!config_get_##type(config,key,&var))                                                            \
        {                                                                                                  \
            fprintf(stderr,"Key: %s not found. Using defalut value\n",key);                                \
            var = default_val;                                                                             \
        }                                                                                                  \
    }while(0)

    Config* config = config_from_file(cofig_file);

    if(!config) return NULL; 

    with_default(int,"window_width",cfg->window_width,
                 CHAKSU_WINDOW_WIDTH);
    with_default(int,"window_height",cfg->window_height,
                 CHAKSU_WINDOW_HEIGHT);
    with_default(int,"frame_rate",cfg->chaksu_framerate,
                 CHAKSU_FRAMERATE);
    with_default(int,"font_size",cfg->chaksu_message_font_size,
                 CHAKSU_MESSAGE_FONT_SIZE);

    with_default(string,"font_path",cfg->font_path,
                 NULL);

    with_default(float,"scale_factor",cfg->chaksu_scale_factor,
                 CHAKSU_SCALE_FACTOR);
    with_default(float,"min_scale",cfg->chaksu_min_scale,
                 CHAKSU_MIN_SCALE);

    with_default(color,"background_color",cfg->chaksu_bg_color,
                 CHAKSU_BG_COLOR);
    with_default(color,"message_color",cfg->chaksu_message_color,
                 CHAKSU_MESSAGE_COLOR);
    with_default(color,"message_error_color", cfg->chaksu_message_err_color,
                 CHAKSU_MESSAGE_ERR_COLOR);

    with_default(keyboard_key,"key_next_image", cfg->chaksu_next_image,
                 CHAKSU_NEXT_IMAGE);
    with_default(keyboard_key,"key_prev_image", cfg->chaksu_next_image,
                 CHAKSU_PREV_IMAGE);
    with_default(keyboard_key,"key_rotate_ccw", cfg->chaksu_rotate_ccw,
                 CHAKSU_ROTATE_CCW);
    with_default(keyboard_key,"key_rotate_cw", cfg->chaksu_rotate_cw,
                 CHAKSU_ROTATE_CW);
    with_default(keyboard_key,"key_zoom_reset", cfg->chaksu_fit_screen,
                 CHAKSU_FIT_SCREEN);
    return config;
}

int main(int argc, char **argv)
{
    chaksu_arguments passed_args = parse_argument((const char**)argv,argc); 
    Config* config;

    if(passed_args.config_file && IsPathFile(passed_args.config_file))
    {
        puts(passed_args.config_file);
        config = chaksu_load_config(passed_args.config_file,&default_config); 
    }
    else
    {
        // load config file from standard config dir or current dir
    }
   
    
    Texture2D texture; 
    char **images      = NULL;
    bool dragging      = false;
    Vector2 offset     = {0, 0};
    float last_click   = 0;
    int total_images   = 0;
    int window_width   = default_config.window_width;
    int current_image  = -1;
    int window_height  = default_config.window_height;
    Vector2 image_pos  = {0, 0};
    char message[2048] = {0};
    float target_scale = 1.0f;

    #ifndef RELEASE
        SetTraceLogLevel(LOG_NONE); 
    #endif

    if(passed_args.other_arguments)
    {
        images = get_all_valid_images(passed_args.other_arguments,
                                      vector_length(passed_args.other_arguments),
                                      false
                                      );
    }
    else
    {
        images = get_images_from_dir(GetWorkingDirectory(),false);
    }

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(window_width, window_height, WINDOW_TITLE);
    SetTargetFPS(default_config.chaksu_framerate);

    // https://www.reddit.com/r/raylib/comments/1i40fxp/comment/m7thpjr/?utm_source=share&utm_medium=web3x&utm_name=web3xcss&utm_term=1&utm_content=share_button
    EnableEventWaiting();

    if (images && (total_images = vector_length(images)) > 0)
    {
        texture   = chaksu_load_texture(images[++current_image]);
        image_pos = update_pos(texture, &target_scale);
    }

    const Vector2 dpi_scale     = GetWindowScaleDPI();
    const int message_font_size = (int)ceilf(default_config.chaksu_message_font_size*dpi_scale.y);

    Font custom_font = {0};
    if(!default_config.font_path)
        custom_font = LoadFontFromMemory(".ttf",font_data,
                                         sizeof(font_data),
                                         message_font_size,
                                         0,
                                         0);
    else
        custom_font = LoadFontEx(default_config.font_path,message_font_size, 0,0);

    int angle = 0;

    while (!WindowShouldClose())
    {
        if (IsFileDropped())
        {
            FilePathList droped_files = LoadDroppedFiles();
            char **temp  = get_all_valid_images((const char**)droped_files.paths,
                                                droped_files.count,false); 
            int temp_len = vector_length(temp); 

            for(int i = 0 ; i < temp_len; i++)
            {
                vector_append(images,temp[i]);
            }

            total_images = vector_length(images); 

            if(current_image == -1 && total_images > 0)
            {
                texture   = chaksu_load_texture(images[++current_image]);
                image_pos = update_pos(texture, &target_scale);
            }

            free_vector(temp); 
            UnloadDroppedFiles(droped_files); 
        }

        if (IsKeyReleased(default_config.chaksu_next_image)&&
            current_image + 1 < total_images)
        {
            UnloadTexture(texture);
            texture   = chaksu_load_texture(images[++current_image]);
            image_pos = update_pos(texture, &target_scale);
            angle = 0;
        }

        if (IsKeyReleased(default_config.chaksu_rotate_cw))
        {
            angle += 90;
            if (angle == 360)
                angle = 0;
        }

        if (IsKeyReleased(default_config.chaksu_rotate_ccw))
        {
            angle -= 90;
            if (angle == -360)
                angle = 0;
        }

        if (IsKeyReleased(default_config.chaksu_fit_screen))
        {
            image_pos = update_pos(texture, &target_scale);
            angle     = 0;
        }

        if (IsKeyReleased(default_config.chaksu_prev_image)
            && current_image - 1 >= 0)
        {
            UnloadTexture(texture);
            texture   = chaksu_load_texture(images[--current_image]);
            image_pos = update_pos(texture, &target_scale);
            angle = 0;
        }

        if (IsGestureDetected(GESTURE_DOUBLETAP)|| 
            IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)||
            IsWindowResized()
           )
        {
            image_pos     = update_pos(texture, &target_scale);
            window_width  = GetScreenWidth();
            window_height = GetScreenHeight();
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            float current_time = GetTime();

            if (current_time - last_click < 0.3f)
                image_pos = update_pos(texture, &target_scale);

            last_click = current_time;

            Vector2 mouse_position = GetMousePosition();

            offset = (Vector2){mouse_position.x - image_pos.x,
                                mouse_position.y - image_pos.y
                              };
            dragging = true;
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
            dragging = false;
        }

        if (dragging)
        {
            Vector2 mouse_position = GetMousePosition();
            image_pos = (Vector2){
                .x = mouse_position.x - offset.x,
                .y = mouse_position.y - offset.y,
            };
        }

        float scroll = GetMouseWheelMove();
        if (scroll != 0.0f)
        {
            Vector2 mouse_position  = GetMousePosition();
            Vector2 mouse_world_pos = {(mouse_position.x - image_pos.x) / target_scale,
                                       (mouse_position.y - image_pos.y) / target_scale};

            target_scale += scroll * default_config.chaksu_scale_factor;

            if (target_scale < default_config.chaksu_min_scale)
                target_scale = default_config.chaksu_min_scale;

            image_pos.x = mouse_position.x - mouse_world_pos.x * target_scale;
            image_pos.y = mouse_position.y - mouse_world_pos.y * target_scale;
        }


        BeginDrawing();
        ClearBackground(default_config.chaksu_bg_color);

        if(total_images > 0)
        {
            update_message(message, "[%d/%d](zoom %.2f%%) %s",
                           current_image + 1,
                           total_images,
                           target_scale * 100,
                           images[current_image]
                           );

            BeginScissorMode(0, 0, window_width, window_height - OFFSET);

            Rectangle source      = {0, 0, texture.width, texture.height};
            Vector2 origin        = {(texture.width * target_scale) / 2,
                                     (texture.height * target_scale) / 2
                                    };
            Rectangle destination = {
                image_pos.x + origin.x,
                image_pos.y + origin.y,
                texture.width * target_scale,
                texture.height * target_scale
            };

            DrawTexturePro(texture, source, destination, origin,(float)angle, WHITE);

            EndScissorMode();
            DrawTextEx(custom_font, message,
                       (Vector2){0, window_height - (OFFSET + message_font_size) / 2.0f},
                       message_font_size, 
                       1, 
                       default_config.chaksu_message_color
                       );
        }
        else
        {
            update_message(message, "%s",
                           "Drag and Drop image(s) file or Folder containing image(s)");
            DrawTextEx(custom_font,
                       message,
                       (Vector2){20, window_height - OFFSET},
                       message_font_size,
                       1,
                       default_config.chaksu_message_err_color
                    );
        }

        EndDrawing();
        
    }

//cleanup: unused label
    total_images = vector_length(images);

    for (int i = 0; i < total_images; i++)
        free(images[i]);

    free_vector(images);
    free_vector(passed_args.other_arguments);
    config_free(config);
    UnloadTexture(texture);
    CloseWindow();

    return 0;
}
