#include <string.h>

#include <RtspServer.hpp>
#include <RtspProxyConfig.hpp>

namespace rtsp_proxy_server {

RtspServer::RtspServer(
    int argc,
    char** argv) {

    if (argc > 1) {
        m_config = std::make_shared<RtspProxyConfig>(argv[1]);
    } else {
        m_config = std::make_shared<RtspProxyConfig>();
    }

    gst_init(&argc, &argv);

    // Create an instance of the RTSP server
    m_server = gst_rtsp_server_new();

    //g_object_set(m_server, "service", m_port.c_str(), NULL);
    GstRTSPMountPoints* mounts = gst_rtsp_server_get_mount_points(m_server);

    m_factory = gst_rtsp_media_factory_new();

    gst_rtsp_media_factory_set_launch(
        m_factory,
        m_config->getOutputPipeline().c_str());

    gst_rtsp_media_factory_set_shared(m_factory, TRUE);

    auto id = g_signal_connect(
        m_server,
        "client-connected",
        G_CALLBACK(&RtspServer::onClientConnected),
        static_cast<gpointer>(this));
    if (id <= 0) {
        throw std::runtime_error(
            "ERROR: failed to connect 'client-connected' signal to 'server'");
    }

    gst_rtsp_mount_points_add_factory(
        mounts,
        m_config->getOutputPath().c_str(),
        m_factory);

    /* don't need the ref to the mapper anymore */
    g_object_unref(mounts);

    g_print("Added mount point '%s'\n", m_config->getOutputPath().c_str());
    g_print("GStreamer pipeline is:\n\t'%s'\n",
        m_config->getOutputPipeline().c_str());
}

void
RtspServer::run()
{
    // Create the main gstreamer loop
    auto loop = g_main_loop_new(NULL, FALSE);

    // Attach the server to the default context
    if (gst_rtsp_server_attach(m_server, NULL) == 0) {
        throw std::runtime_error(
            "RTSP server failed to attach. "
            "Is another instance already running?");
    }

    g_print("\nRtspServer started\n");
    g_main_loop_run(loop);

    g_main_loop_unref(loop);
    g_print("\nRtspServer STOPPED\n");
}

void
RtspServer::onClientConnected(GstRTSPServer*,
    GstRTSPClient* gstClient,
    RtspServer* rtspProxyServer)
{
    g_print("gst client-connected: %p\n", gstClient);

    auto client = new RtspClient(
        gstClient,
        reinterpret_cast<GCallback>(&RtspServer::onClientDisconnected),
        rtspProxyServer);

    auto res = rtspProxyServer->m_clients.emplace(gstClient, client);

    g_print("client %p is added to the list: res=%d\n", gstClient, res.second);
}

void RtspServer::onClientDisconnected(GstRTSPClient* gstClient,
    RtspServer* rtspProxyServer)
{
    g_printerr("Got a disconnect from client %p", gstClient);

    auto it = rtspProxyServer->m_clients.find(gstClient);
    if (it == rtspProxyServer->m_clients.end()) {
        g_printerr("Could not locate the disconnecting client!");
    } else {
        g_print("deleting client %p\n", it->second);
        delete it->second;
        rtspProxyServer->m_clients.erase(it);
        g_print("client disconnected.\n");
    }
}

} // end of namespace
