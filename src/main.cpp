#include "utils.h"
#include <string>
#include <chrono>
#include <thread>
#include <csignal>
#include <atomic>
#include <opencv2/opencv.hpp>

std::string g_temp_file;
bool g_is_temp = false;
std::atomic<bool> g_terminate{false};

void cleanup_signal(int signum) { g_terminate = true; }

int main(int argc, char** argv)
{
    std::signal(SIGINT, cleanup_signal);
    std::signal(SIGTERM, cleanup_signal);

#ifdef _WIN32
    BOOL WINAPI consoleHandler(DWORD signal)
    {
        if (signal == CTRL_C_EVENT || signal == CTRL_BREAK_EVENT)
            g_terminate = true;
        return TRUE;
    }
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

    g_is_temp = handle_url(filepath, temp_file);
    cv::VideoCapture capture(filepath);
    if (!capture.isOpened()) 
    {
        std::cerr << "Error: Could not open video.\n";
        if (g_is_temp) std::remove(temp_file.c_str());
        return 1;
    }

    long frametime = (argc > 2) ?
        static_cast<long>(1e6 / std::stoi(argv[2])) :
        static_cast<long>(1e6 / capture.get(cv::CAP_PROP_FPS));
    if (frametime <= 0) frametime = 33333;

    cv::Mat frame, gray, small;
    while (true) 
    {
        if (g_terminate) break;
        auto start = std::chrono::high_resolution_clock::now();
        capture >> frame;
        if (frame.empty()) break;

        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

        Res term = get_term_size();
        Res out  = fit_to_term(frame.cols, frame.rows, term.x, term.y);

        cv::resize(gray, small, cv::Size(out.x, out.y));

        render_ascii(small, ASCIIcharset);

        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        if (frametime > elapsed.count())
            std::this_thread::sleep_for(std::chrono::microseconds(frametime - elapsed.count()));
    }
    if (g_is_temp)
    {
        std::remove(temp_file.c_str());
        std::cout << "Temporary file removed.";
    }
}
