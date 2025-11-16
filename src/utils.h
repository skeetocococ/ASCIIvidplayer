#pragma once
#include <string>
#include <opencv2/opencv.hpp>

struct Res { int x, y; };

Res get_term_size();
Res fit_to_term(int vid_w, int vid_h, int term_cols, int term_rows);
char pixel_to_char(int value, std::string charset);
void render_ascii(const cv::Mat& small, const std::string& charset);
bool handle_url(std::string& path, std::string& temp_file);
