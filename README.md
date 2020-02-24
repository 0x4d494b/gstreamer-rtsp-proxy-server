This project creates a gstreamer/opencv RTSP proxy server using Gstreamer RTSP server API and OpenCV 4.2 API. 
The proxy opens RTSP streams specified in the config file (config/rtsp-proxy.yaml), and creates its own 
RTSP server that processes and combines all incoming streams into a single stream.

The server creates one OpenCV reader thread per input RTSP stream. And it creates one ProxyProcessor 
thread that reads the frames from each input stream and produces a new, outgoing, video frame. 
The gstreamer RTSP server's "need-data" callback simply copies that new video frame into the 
outgoing gstreamer pipeline, so a connecting RTSP client could display it.

The server should be able to handle multiple clients, and it only opens input streams when 
a client connects to output side of the proxy.

Note that gstreamer is used from openCV to open input frames. OpenCV is capable opening 
RTSP on its own, but I couldn't find a way to turn the buffering off. 
Otherwise, it works exactly the same. If latency is important, further optimization can 
be done using GPU/CUDA API.

Note that this is just an example of creating RTSP proxy, and dealing with each 
incoming video frame from OpenCV. To make this code more generic, it would need 
to implement a plugin for specific proxy processing. For example, a seamless 
panorama stiching, or birds eye view, etc.

To build:
---------

mkdir build
cd build
cmake ..
make

To run
------
./rtsp-proxy-server ../config/rtsp-proxy.yaml
