#include <string.h>
#include <math.h>
#include <stdio.h>

#include <webp/decode.h>

#include "raylib.h"

#define IMPLEMENT_VECTOR
#include "vector.h"

#include "./config.h"

#if !defined (CHAKSU_CUSTOM_FONT)
    #include "resources.c"
#endif


#define WINDOW_TITLE "Chaksu Image Viewer"
#define OFFSET 50
#define update_message(message, fmt, ...) snprintf(message, sizeof(message), fmt, __VA_ARGS__)

// https://www.reddit.com/r/C_Programming/comments/1i40cus/comment/m7tryqu/?utm_source=share&utm_medium=web3x&utm_name=web3xcss&utm_term=1&utm_content=share_button

#define MAX_FILE_EXTENSION_LEN 16
const char valid_extensions[][MAX_FILE_EXTENSION_LEN] = {
    ".png", ".jpg", ".jpeg", ".gif", ".psd", ".tga", ".bmp", ".ppm",
     ".pic", ".hdr", ".pvr",  ".qoi", ".dds", ".pkm", ".ktx", ".astc",".webp"
};


const int total_extensions = sizeof(valid_extensions) / sizeof(valid_extensions[0]);

char *str_duplicate(const char *str)
{
    size_t len = strlen(str);
    char *dulicated_str = malloc(len + 1);

    if (!dulicated_str)
        return NULL;

    memcpy(dulicated_str, str, len);
    dulicated_str[len] = '\0';

    return dulicated_str;
}

Vector2 update_pos(Texture2D texture, float *scale)
{
    const int screen_width   = GetScreenWidth();
    const int screen_height  = GetScreenHeight() - OFFSET;

    if(texture.width <= screen_width && texture.height <= screen_height)
    {
        *scale = 1;
        return (Vector2){(screen_width - texture.width) / 2.0, (screen_height - texture.height) / 2.0};

    }

    const float aspect_ratio = (float)texture.width / texture.height;

    int new_width  = screen_width;
    int new_height = new_width / aspect_ratio;

    if (new_height > screen_height)
    {
        new_height = screen_height;
        new_width  = screen_height * aspect_ratio;
    }

    *scale = (float)new_width / texture.width;

    return (Vector2){(screen_width - new_width) / 2.0, (screen_height - new_height) / 2.0};
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

char **get_images_from_dir__helper(char ***result, const char *dir)
{
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

    for (int i = 0; i < files.count; i++)
    {
        if (is_image(files.paths[i]))
            vector_append(*tmp, str_duplicate(files.paths[i]));
    }

    UnloadDirectoryFiles(files);

    return *tmp;
}
char **get_images_from_dir(const char *dir)
{
    char **images = get_images_from_dir__helper(NULL, dir);

    if (!images)
        return NULL;

    return images;
}

char **parser_argument(const char **files, const int n)
{
    if (n < 1)
        return NULL;

    char **images = Vector(*images);

    if (!images)
        return NULL;

    for (int i = 0; i < n; i++)
    {
        if (IsPathFile(files[i]))
        {
            if (is_image(files[i]))
                vector_append(images, str_duplicate(files[i]));
        }
        else
        {
            (void)get_images_from_dir__helper(&images, files[i]);
        }
    }

    return images;
}

Texture chaksu_load_texture(const char *file)
{
    Texture texture = {0};
    if(!is_image(file)) return texture;

    if(IsFileExtension(file, ".webp"))
    {
            int data_size = 0;
            int width = 0;
            int height = 0;
            unsigned char *file_data = LoadFileData(file, &data_size);

            if(data_size == 0) return texture;

            uint8_t  *pixels = WebPDecodeRGBA(file_data, data_size,&width, &height);

            if(!pixels || height == 0 || width == 0) return texture;

            Image img = {.data = pixels,
                         .mipmaps = 1,
                         .width = width,
                         .height = height,
                         .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
                        };
            texture = LoadTextureFromImage(img); 
            UnloadImage(img);
            return texture; 

    }
    return LoadTexture(file);
}


int main(int argc, char **argv)
{
    Texture2D texture;

    char **images      = NULL;
    bool dragging      = false;
    Vector2 offset     = {0, 0};
    float last_click   = 0;
    int total_images   = 0;
    int window_width   = CHAKSU_WINDOW_WIDTH;
    int current_image  = -1;
    int window_height  = CHAKSU_WINDOW_HEIGHT;
    Vector2 image_pos  = {0, 0};
    char message[2048] = {0};
    float target_scale = 1.0f;

    #ifdef RELEASE
        SetTraceLogLevel(LOG_NONE); 
    #endif

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(window_width, window_height, WINDOW_TITLE);
    SetTargetFPS(CHAKSU_FRAMERATE);

    // https://www.reddit.com/r/raylib/comments/1i40fxp/comment/m7thpjr/?utm_source=share&utm_medium=web3x&utm_name=web3xcss&utm_term=1&utm_content=share_button
    EnableEventWaiting();

    if (argc == 1){
        images = get_images_from_dir(GetWorkingDirectory());

    }
    else
        images = parser_argument((const char **)argv + 1, argc - 1);

    if (images && (total_images = vector_length(images)) > 0)
    {
        texture   = chaksu_load_texture(images[++current_image]);
        image_pos = update_pos(texture, &target_scale);
    }

    const Vector2 dpi_scale     = GetWindowScaleDPI();
    const int message_font_size = (int)ceilf(CHAKSU_MESSAGE_FONT_SIZE * dpi_scale.y);

    #if !defined (CHAKSU_CUSTOM_FONT)
        Font custom_font = LoadFontFromMemory(".ttf", font_data, sizeof(font_data), message_font_size, 0, 0);
    #else
        Font custom_font = LoadFont(CHAKSU_CUSTOM_FONT);
    #endif

    int angle = 0;

    while (!WindowShouldClose())
    {
        if (IsFileDropped())
        {
            FilePathList droped_files = LoadDroppedFiles();
            char **temp  = parser_argument((const char**)droped_files.paths,droped_files.count); 
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

        if (IsKeyReleased(CHAKSU_NEXT_IMAGE) && current_image + 1 < total_images)
        {
            UnloadTexture(texture);
            texture   = chaksu_load_texture(images[++current_image]);
            image_pos = update_pos(texture, &target_scale);
            angle = 0;
        }

        if (IsKeyReleased(CHAKSU_ROTATE_CW))
        {
            angle += 90;
            if (angle == 360)
                angle = 0;
        }

        if (IsKeyReleased(CHAKSU_ROTATE_CCW))
        {
            angle -= 90;
            if (angle == -360)
                angle = 0;
        }

        if (IsKeyReleased(CHAKSU_FIT_SCREEN))
        {
            image_pos = update_pos(texture, &target_scale);
            angle     = 0;
        }

        if (IsKeyReleased(CHAKSU_PREV_IMAGE) && current_image - 1 >= 0)
        {
            UnloadTexture(texture);
            texture   = chaksu_load_texture(images[--current_image]);
            image_pos = update_pos(texture, &target_scale);
            angle = 0;
        }

        if (IsGestureDetected(GESTURE_DOUBLETAP) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || IsWindowResized())
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

            offset = (Vector2){mouse_position.x - image_pos.x, mouse_position.y - image_pos.y};
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

            target_scale += scroll * CHAKSU_SCALE_FACTOR;

            if (target_scale < CHAKSU_MIN_SCALE)
                target_scale = CHAKSU_MIN_SCALE;

            image_pos.x = mouse_position.x - mouse_world_pos.x * target_scale;
            image_pos.y = mouse_position.y - mouse_world_pos.y * target_scale;
        }


        BeginDrawing();
        ClearBackground(CHAKSU_BG_COLOR);

        if(total_images > 0)
        {
            update_message(message, "[%d/%d](zoom %.2f%%) %s", current_image + 1, total_images, target_scale * 100,
                       images[current_image]);

            BeginScissorMode(0, 0, window_width, window_height - OFFSET);

            Rectangle source      = {0, 0, texture.width, texture.height};
            Vector2 origin        = {(texture.width * target_scale) / 2, (texture.height * target_scale) / 2};
            Rectangle destination = {image_pos.x + origin.x, image_pos.y + origin.y, texture.width * target_scale,
                                     texture.height * target_scale};

            DrawTexturePro(texture, source, destination, origin,(float)angle, WHITE);

            EndScissorMode();
            DrawTextEx(custom_font, message, (Vector2){0, window_height-(OFFSET+message_font_size)/2.0f}, message_font_size, 1,CHAKSU_MESSAGE_COLOR);

        }
        else
        {
            update_message(message, "%s","Drag and Drop image(s) file or Folder containing image(s)"); 
            DrawTextEx(custom_font, message, (Vector2){20, window_height - OFFSET}, message_font_size, 1,CHAKSU_MESSAGE_ERR_COLOR);

        }

        EndDrawing();
        
    }

cleanup:
    total_images = vector_length(images);

    for (int i = 0; i < total_images; i++)
        free(images[i]);

    free_vector(images);
    UnloadTexture(texture);
    CloseWindow();

    return 0;
}
