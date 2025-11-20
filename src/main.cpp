#include "utils.h"
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <csignal>
#include <atomic>
#include <opencv2/opencv.hpp>

std::string temp_file;
bool is_temp = false;
std::atomic<bool> terminate{false};

void cleanup_signal(int signum) { ::terminate = true; }

#ifdef _WIN32
#include <windows.h>
BOOL WINAPI consoleHandler(DWORD signal)
{
    if (signal == CTRL_C_EVENT || signal == CTRL_BREAK_EVENT)
        ::terminate = true;
    return TRUE;
}
#endif

int main(int argc, char** argv)
{
    std::signal(SIGINT, cleanup_signal);
    std::signal(SIGTERM, cleanup_signal);
#ifdef _WIN32
    SetConsoleCtrlHandler(consoleHandler, TRUE);
#endif

    if (argc < 2) 
    {
        std::cout << "Usage: " << argv[0] << " <video_file or URL> [fps]\n";
        return 1;
    }

    std::string filepath = argv[1];
    std::string temp_file;
    std::string ASCIIcharset = " .`^\",:;Il!i~+_-?][}{1)(|\\/*tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";
    std::string render_mode;
    float gamma = 2.2;
    double aspect = 0.5;
    double start = 0.0;
    bool loop;

    ::is_temp = handle_url(filepath, temp_file);
    cv::VideoCapture capture(filepath);
    if (!capture.isOpened()) 
    {
        std::cerr << "Error: Could not open video.\n";
        if (::is_temp) std::remove(temp_file.c_str());
        return 1;
    }

    long frametime = static_cast<long>(1e6 / capture.get(cv::CAP_PROP_FPS));
    parse_cli(argc, argv, frametime, gamma, render_mode, aspect, start, loop);
    if (frametime <= 0) frametime = 33333;
    if (start > 0) capture.set(cv::CAP_PROP_POS_MSEC, start * 1000);

    cv::Mat frame, gray, small;
    while (true) 
    {
        if (::terminate) break;
        auto start = std::chrono::high_resolution_clock::now();
        capture >> frame;
        if (frame.empty()) break;

        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

        Res term = get_term_size();
        Res out  = fit_to_term(frame.cols, frame.rows, term.x, term.y, aspect);

        if (render_mode == "half")
        {
            cv::resize(frame, small, cv::Size(out.x, out.y*2), 0, 0, cv::INTER_AREA);
            render_halfblock(small);
        }
        else
        { 
            cv::resize(gray, small, cv::Size(out.x, out.y*2), 0, 0, cv::INTER_AREA);
            render_ascii(small, ASCIIcharset, gamma);
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        if (frametime > elapsed.count())
            std::this_thread::sleep_for(std::chrono::microseconds(frametime - elapsed.count()));
    }
    if (::is_temp)
    {
        std::remove(temp_file.c_str());
        std::cout << "Temporary file removed.";
    }
}
