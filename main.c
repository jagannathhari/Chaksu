#include <string.h>

#include "raylib.h"

#define IMPLEMENT_VECTOR
#include "vector.h"

#include <math.h>
#include <stdio.h>

#include "resources.c"

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 700
#define WINDOW_TITLE "Chaksu Image Viewer"
#define BACKGROUND_COLOR (Color){0x28,0x28,0x28, 255}
#define OFFSET 50
#define MESSAGE_FONT_SIZE 20
#define update_message(message, fmt, ...) snprintf(message, sizeof(message), fmt, __VA_ARGS__)

const char valid_extesnsions[][5] = {".png", ".jpg", ".jpeg", ".gif", ".psd", ".tga", ".bmp", ".ppm",
                                     ".pic", ".hdr", ".pvr",  ".qoi", ".dds", ".pkm", ".ktx", ".astc"};
const int total_extesnions = sizeof(valid_extesnsions) / sizeof(valid_extesnsions[0]);

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

    for (int i = 0; i < total_extesnions; i++)
    {
        if (IsFileExtension(path, valid_extesnsions[i]))
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

int main(int argc, char **argv)
{
    Texture2D texture;

    char **images      = NULL;
    bool dragging      = false;
    Vector2 offset     = {0, 0};
    float last_click   = 0;
    int total_images   = 0;
    int window_width   = WINDOW_WIDTH;
    int current_image  = -1;
    int window_height  = WINDOW_HEIGHT;
    Vector2 image_pos  = {0, 0};
    char message[2048] = {0};
    float target_scale = 1.0f;

    #ifdef RELEASE
        SetTraceLogLevel(LOG_NONE); 
    #endif

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(window_width, window_height, WINDOW_TITLE);
    SetTargetFPS(60);

    if (argc == 1){
        images = get_images_from_dir(GetWorkingDirectory());

    }
    else
        images = parser_argument((const char **)argv + 1, argc - 1);

    if (images && (total_images = vector_length(images)) > 0)
    {
        texture   = LoadTexture(images[++current_image]);
        image_pos = update_pos(texture, &target_scale);
    }

    const Vector2 dpi_scale     = GetWindowScaleDPI();
    const int message_font_size = (int)ceilf(MESSAGE_FONT_SIZE * dpi_scale.y);

    Font customFont = LoadFontFromMemory(".ttf", font_data, sizeof(font_data), message_font_size, 0, 0);

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
                texture   = LoadTexture(images[++current_image]);
                image_pos = update_pos(texture, &target_scale);
            }

            free_vector(temp); 
            UnloadDroppedFiles(droped_files); 
        }

        if (IsKeyReleased(KEY_N) && current_image + 1 < total_images)
        {
            UnloadTexture(texture);
            texture   = LoadTexture(images[++current_image]);
            image_pos = update_pos(texture, &target_scale);
            angle = 0;
        }

        if (IsKeyReleased(KEY_A))
        {
            angle += 90;
            if (angle == 360)
                angle = 0;
        }

        if (IsKeyReleased(KEY_S))
        {
            angle -= 90;
            if (angle == -360)
                angle = 0;
        }

        if (IsKeyReleased(KEY_ZERO))
        {
            image_pos = update_pos(texture, &target_scale);
            angle     = 0;
        }

        if (IsKeyReleased(KEY_P) && current_image - 1 >= 0)
        {
            UnloadTexture(texture);
            texture   = LoadTexture(images[--current_image]);
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

            target_scale += scroll * 0.3f;

            if (target_scale < 0.1f)
                target_scale = 0.1f;

            image_pos.x = mouse_position.x - mouse_world_pos.x * target_scale;
            image_pos.y = mouse_position.y - mouse_world_pos.y * target_scale;
        }


        BeginDrawing();
        ClearBackground(BACKGROUND_COLOR);

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
            DrawTextEx(customFont, message, (Vector2){0, window_height-(OFFSET+message_font_size)/2.0f}, message_font_size, 1, WHITE);

        }
        else
        {
            update_message(message, "%s","Drag and Drop image(s) file or Folder containing image(s)"); 
            DrawTextEx(customFont, message, (Vector2){20, window_height - OFFSET}, message_font_size, 1,RED);

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
