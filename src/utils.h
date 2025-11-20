#pragma once
#include <string>
#include <opencv2/opencv.hpp>

struct Res { int x, y; };

struct Settings
{
    std::string ASCIIcharset = " .`^\",:;Il!i~+_-?][}{1)(|\\/*tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";
    std::string temp_file;
    std::string render_mode;
    float gamma = 2.2;
    double aspect = 0.3;
    double start = 0.0;
    bool loop = false;
};

void parse_cli(int count, char** args, long& frametime, float& gamma, std::string& render_mode, std::string& charset, double& char_aspect, double& start, bool& loop);
Res get_term_size();
Res fit_to_term(int vid_w, int vid_h, int term_cols, int term_rows, double char_aspect);
void render(const std::string& render_mode, const std::string& charset, cv::Mat& small, cv::Mat& frame, cv::Mat& gray, Res& out, float& gamma);
bool handle_url(std::string& path, std::string& temp_file);
