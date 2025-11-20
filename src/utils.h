#pragma once
#include <string>
#include <opencv2/opencv.hpp>

struct Res { int x, y; };

void parse_cli(int count, char** args, long& frametime, float& gamma, std::string& render_mode, double& char_aspect, double& start, bool& loop);
Res get_term_size();
Res fit_to_term(int vid_w, int vid_h, int term_cols, int term_rows, double char_aspect);
void render_halfblock(const cv::Mat& img);
void render_ascii(const cv::Mat& small, const std::string& charset, float gamma); 
bool handle_url(std::string& path, std::string& temp_file);
