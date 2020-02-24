#ifndef RTSP_PROXY_RTSP_SERVER_HPP
#define RTSP_PROXY_RTSP_SERVER_HPP

#include <unordered_map>

// project headers
#include <RtspProxyConfig.hpp>
#include <RtspClient.hpp>

namespace rtsp_proxy_server {

class RtspServer {
public:
    /**
     * \brief Constructor
     *
     * Create an RtspServer object.
     */
    RtspServer(int argc, char** argv);

    /**
     * \brief Start RTSP server
     */
    void run();

    /**
     * \brief Get gstreamer RTSP media factory created by this instance
     *        of RTSP server
     */
    GstRTSPMediaFactory* getFactory() { return m_factory; }

    /**
     * \brief Get RTSP proxy server configuration
     */
    std::shared_ptr<RtspProxyConfig> getConfig() { return m_config; }

private:
    /**
     * \brief Callback for RTSP client connecting to our RTSP server
     *
     * \param[in] gstServer gstreamer RTSP server object
     * \param[in] gstClient gstreamer RTSP client object
     * \param[in] rtspProxyServer pointer to our proxy RtspServer
     */
    static void onClientConnected(
        GstRTSPServer* gstServer,
        GstRTSPClient* gstClient,
        RtspServer* rtspProxyServer);

    /**
     * \brief Callback for RTSP client disconnecting from our RTSP server
     *
     * \param[in] gstClient gstreamer RTSP client object
     * \param[in] rtspProxyServer pointer to our proxy RtspServer
     */
    static void onClientDisconnected(
        GstRTSPClient* gstClient,
        RtspServer* rtspProxyServer);

private:
    /**
     * RTSP proxy server and remote cameras configuration
     */
    std::shared_ptr<RtspProxyConfig> m_config;

    /**
     * This is the underlying GStreamer RTSP server instance
     */
    GstRTSPServer* m_server = nullptr;

    /** GStreamer RTSP media factory used by the RTSP server */
    GstRTSPMediaFactory* m_factory = nullptr;

    /** map of all connected RTSP clients */
    std::unordered_map<GstRTSPClient*,RtspClient*> m_clients;
};

} // end of namespace

#endif
