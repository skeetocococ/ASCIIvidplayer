#include "utils.h"
#include <iostream>
#include <math.h>
#include <opencv2/opencv.hpp>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/ioctl.h>
#endif

void parse_cli(int count, char** args, long& frametime, float& gamma, 
               std::string& render_mode, double& char_aspect, 
               double& start, bool& loop)
{
    for (int i = 2; i < count; ++i) 
    {
        std::string arg = args[i];
        if (arg == "--fps" && i+1 < count) 
            frametime = static_cast<long>(1e6 / std::stoi(args[++i]));
        if (arg == "--gamma" && i+1 < count) 
            gamma = std::stod(args[++i]);
        if (arg == "--mode" && i+1 < count) 
            render_mode = args[++i]; // "ascii" | "half" | "braille"
        if (arg == "--aspect" && i+1 < count) 
            char_aspect = std::stod(args[++i]);
        if (arg == "--start" && i + 1 < count)
            start = std::stod(args[++i]);
        if (arg == "--loop")
            loop = true;
    }
}

Res get_term_size() 
{
    const Res DEFAULT_SIZE{80, 24};
#ifdef _WIN32
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStdout == INVALID_HANDLE_VALUE) return DEFAULT_SIZE;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(hStdout, &csbi)) return DEFAULT_SIZE;
    int columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    int rows    = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    rows = std::max(1, rows - 1);
    return { columns, rows };
#else
    if (!isatty(STDOUT_FILENO)) return DEFAULT_SIZE;
    struct winsize w{};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) return DEFAULT_SIZE;
    int columns = w.ws_col > 0 ? w.ws_col : DEFAULT_SIZE.x;
    int rows    = w.ws_row > 0 ? w.ws_row - 1 : DEFAULT_SIZE.y;
    return { columns, rows };
#endif
}

Res fit_to_term(int vid_w, int vid_h, int term_cols, int term_rows, double char_aspect)
{
    if (vid_w <= 0 || vid_h <= 0) return {term_cols, term_rows};

    double adj_term_w = term_cols * char_aspect;
    double term_aspect = adj_term_w / term_rows;
    double vid_aspect = static_cast<double>(vid_w) / static_cast<double>(vid_h);

    int out_w, out_h;
    if (term_aspect > vid_aspect) 
    {
        out_h = term_rows;
        out_w = static_cast<int>(std::round(vid_aspect * out_h / char_aspect));
    } else 
    {
        out_w = term_cols;
        out_h = static_cast<int>(std::round(out_w / vid_aspect * char_aspect));
    }
    out_w = std::max(1, out_w);
    out_h = std::max(1, out_h);
    return { out_w, out_h };
}

static char pixel_to_char(unsigned char value, const std::string& charset, float gamma)
{
    float normalized = value / 255.0f;
    float corrected = std::pow(normalized, gamma);
    unsigned char v = static_cast<unsigned char>(corrected * 255);

    const int charset_len = charset.size();
    int index = v * charset_len / 256;
    if (index >= charset_len) index = charset_len - 1;
    return charset[index];
}

void render_halfblock(const cv::Mat& img)
{
    std::string out;
    out.reserve((img.cols + 2) * (img.rows/2));
    out += "\033[H";

    int h = img.rows;
    int w = img.cols;
    for (int y = 0; y < h; y += 2)
    {
        const cv::Vec3b* topRow    = img.ptr<cv::Vec3b>(y);
        const cv::Vec3b* bottomRow = img.ptr<cv::Vec3b>(std::min(y+1, h-1));
        for (int x = 0; x < w; ++x)
        {
            auto top = topRow[x];     
            auto bot = bottomRow[x];
            int tr = top[2], tg = top[1], tb = top[0];
            int br = bot[2], bg = bot[1], bb = bot[0];
            out += "\033[38;2;" + std::to_string(tr) + ";" + std::to_string(tg) + ";" + std::to_string(tb) + "m";
            out += "\033[48;2;" + std::to_string(br) + ";" + std::to_string(bg) + ";" + std::to_string(bb) + "m";
            out += "▀"; // upper half block - shows foreground color on top half
        }
        out += "\033[0m\033[K\n"; // reset colors and clear rest of line
    }
    std::cout << out;
}

void render_ascii(const cv::Mat& small, const std::string& charset, float gamma) 
{
    std::string buffer;
    buffer.reserve((small.cols + 2) * small.rows);
    buffer += "\033[H";
    for (int y = 0; y < small.rows; ++y) 
    {
        const unsigned char* row = small.ptr<unsigned char>(y);
        for (int x = 0; x < small.cols; ++x)
        {
            unsigned char v = row[x];
            char c = pixel_to_char(v, charset, gamma);

            int color = 232 + v * 23 / 255;
            buffer += "\033[38;5;" + std::to_string(color) + "m";
            buffer += c;
        }
        buffer += "\033[0m\033[K\n";
    }
    std::cout << buffer;
}

bool handle_url(std::string& path, std::string& temp_file)
{
    if (path.compare(0, 7, "http://") == 0 || path.compare(0, 8, "https://") == 0)
    {
        temp_file = "temp_video.mp4";
        std::string cmd = "yt-dlp -f mp4 \"" + path + "\" -o " + temp_file;
        int ret = std::system(cmd.c_str());
        if (ret != 0)
        {
            std::cerr << "Failed to download video.\n";
            exit(EXIT_FAILURE);
        }
        path = temp_file;
        return true;
    }
    return false;
}
