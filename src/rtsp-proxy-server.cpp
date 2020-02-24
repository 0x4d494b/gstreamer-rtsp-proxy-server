#include <RtspServer.hpp>

using namespace rtsp_proxy_server;

int main(int argc, char** argv)
{
    try {
        // Run the server object
        RtspServer server(argc, argv);
        server.run();
    } catch (std::exception const& e) {
        fprintf(stderr, "\n\nERROR: RTSP Proxy Server failed: %s\n\n",
            e.what());
        return 1;
    }
    return 0;
}
