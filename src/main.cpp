#include "opencv2/imgproc.hpp"
#include "utils.h"
#include <string>
#include <chrono>
#include <thread>
#include <csignal>
#include <atomic>
#include <opencv2/opencv.hpp>

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
    Settings s;

    ::is_temp = handle_url(filepath, s.temp_file);
    cv::VideoCapture capture(filepath);
    if (!capture.isOpened()) 
    {
        std::cerr << "Error: Could not open video.\n";
        if (::is_temp) std::remove(s.temp_file.c_str());
        return 1;
    }

    long frametime = static_cast<long>(1e6 / capture.get(cv::CAP_PROP_FPS));
    parse_cli(argc, argv, frametime, s.gamma, s.render_mode, s.ASCIIcharset, s.aspect, s.start, s.loop);
    if (frametime <= 0) frametime = 33333;
    if (s.start > 0) capture.set(cv::CAP_PROP_POS_MSEC, s.start * 1000);

    cv::Mat frame, gray, small;
    while (true) 
    {
        if (::terminate) break;
        auto start = std::chrono::high_resolution_clock::now();
        capture >> frame;
        if (frame.empty())
        {
            if (s.loop)
            {
                capture.set(cv::CAP_PROP_POS_FRAMES, 0);
                continue;
            }
            break;
        }

        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

        Res term = get_term_size();
        Res out  = fit_to_term(frame.cols, frame.rows, term.x, term.y, s.aspect);

        render(s.render_mode, s.ASCIIcharset, small, frame, gray, out, s.gamma);

        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        if (frametime > elapsed.count())
            std::this_thread::sleep_for(std::chrono::microseconds(frametime - elapsed.count()));
    }
    if (::is_temp)
    {
        std::remove(s.temp_file.c_str());
        std::cout << "Temporary file removed.";
    }
}
