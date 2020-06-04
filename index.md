This project creates a gstreamer/opencv RTSP proxy server using Gstreamer RTSP server API and OpenCV 4.2 API. The proxy opens RTSP streams specified in the config file (config/rtsp-proxy.yaml), and creates its own RTSP server that processes and combines all incoming streams into a single stream.

The server creates one OpenCV reader thread per input RTSP stream. And it creates one ProxyProcessor thread that reads the frames from each input stream and produces a new, outgoing, video frame. The gstreamer RTSP server's "need-data" callback simply copies that new video frame into the outgoing gstreamer pipeline, so a connecting RTSP client could display it.

The server should be able to handle multiple clients, and it only opens input streams when a client connects to output side of the proxy.
