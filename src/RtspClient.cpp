// project headers
#include <RtspServer.hpp>
#include <RtspClient.hpp>

namespace rtsp_proxy_server {

RtspClient::RtspClient(GstRTSPClient* gstClient,
    GCallback onClientDisconnectCallback,
    RtspServer* rtspProxyServer)
{
    m_gstClient = gstClient;
    m_server = rtspProxyServer;

    m_mediaConstructedHandlerId = g_signal_connect(
        rtspProxyServer->getFactory(),
        "media-constructed",
        G_CALLBACK(&RtspClient::onConstructRtspMedia),
        static_cast<gpointer>(this));

    // Let the server know when client disconnects
    m_cliendClosedHandlerId = g_signal_connect(
        gstClient,
        "closed",
        onClientDisconnectCallback,
        rtspProxyServer);
}

RtspClient::~RtspClient()
{
    //
    // disconnect all callbacks when connection closes
    //
    if (m_mediaConstructedHandlerId > 0) {
        g_signal_handler_disconnect(
            m_server->getFactory(), m_mediaConstructedHandlerId);
    }

    if (m_cliendClosedHandlerId > 0) {
        g_signal_handler_disconnect(m_gstClient, m_cliendClosedHandlerId);
    }

    if (m_rtspMedia) {
        delete m_rtspMedia;
    }
}

void
RtspClient::onConstructRtspMedia(
    GstRTSPMediaFactory*,
    GstRTSPMedia* gstRtspMedia,
    RtspClient* client)
{
    client->m_rtspMedia =
        new RtspMedia(gstRtspMedia, client->m_server->getConfig());
}

}
