# ASCIIvidplayer
A simple ASCII video player in C++. Can play video files or URLs in the terminal.

For Unix, Windows, Linux. Uses OpenCV.

Usage
```
(PROGRAM PATH) <video file or URL> [Flags]
```
Target fps are an optional argument that can be passed with `--fps`. If not passed, it will try to find the video's FPS and fall back to 30fps if unsuccessful. Gamma can be set with `--gamma` and the default is 2.2. Using a URL requires that yt-dlp is installed, it would create a temporary file that is deleted when the program is exited or after playback has finished.

Can be built by executing the following in the src directory:
```
mkdir build
cd build
cmake ..
cmake --build .
```
