#pragma once
// 32Blit firmware API
// copied from engine/api_private with types simplified to the minimum

#include <cstdarg>
#include <cstdint>
#include <string>
#include <vector>

namespace blit {

  using AllocateCallback = uint8_t *(*)(size_t);

  enum ScreenMode : uint8_t { lores, hires, hires_palette };

  enum OpenMode {
    read  = 1 << 0,
    write = 1 << 1
  };

  struct Vec2 {
    float x;
    float y;
  };

  struct Vec3 {
    float x;
    float y;
    float z;
  };

  struct Pen {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 0;
  };

  struct Surface {
    uint8_t *data;
  };

  #pragma pack(push, 4)
  struct API {
    uint32_t buttons;
    float hack_left;
    float hack_right;
    float vibration;
    Vec2 joystick;
    Vec3 tilt;
    Pen LED;

    void *channels; // TODO

    Surface &(*set_screen_mode)  (ScreenMode new_mode);
    void (*set_screen_palette)  (const Pen *colours, int num_cols);
    uint32_t (*now)();
    uint32_t (*random)();

    // serial debug
    void (*debug)(std::string message);
    int  (*debugf)(const char * psFormatString, va_list args);

    // files
    void *(*open_file)(std::string file, int mode);
    int32_t (*read_file)(void *fh, uint32_t offset, uint32_t length, char* buffer);
    int32_t (*write_file)(void *fh, uint32_t offset, uint32_t length, const char* buffer);
    int32_t (*close_file)(void *fh);
    uint32_t (*get_file_length)(void *fh);
    //std::vector<FileInfo> (*list_files) (std::string path);
    void *list_files; // TODO
    bool (*file_exists) (std::string path);
    bool (*directory_exists) (std::string path);
    bool (*create_directory) (std::string path);

    // profiler
    void (*enable_us_timer)();
    uint32_t (*get_us_timer)();
    uint32_t (*get_max_us_timer)();

    // jepg - TODO
    //JPEGImage (*decode_jpeg_buffer)(const uint8_t *ptr, uint32_t len, AllocateCallback alloc);
    //JPEGImage (*decode_jpeg_file)(std::string filename, AllocateCallback alloc);

  };
  #pragma pack(pop)

  extern API &api;
}