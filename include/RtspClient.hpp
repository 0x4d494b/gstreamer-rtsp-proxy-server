#ifndef RTSP_PROXY_RTSP_CLIENT_HPP
#define RTSP_PROXY_RTSP_CLIENT_HPP

// project headers
#include <RtspMedia.hpp>

namespace rtsp_proxy_server {

class RtspServer;

class RtspClient {
public:
    /**
     * \brief Constructor
     *
     * Create an RtspClient object.
     *
     * \param[in] gstClient gstreamer RTSP client object that
     *             initiated the connection
     * \param[in] onClientDisconnectCallback RTSP server owned callback to
     *            cleanup RTSP client connection
     * \param[in] rtspProxyServer RTSP server that owns this RtspClient
     */
    RtspClient(
        GstRTSPClient* gstClient,
        GCallback onClientDisconnectCallback,
        RtspServer* rtspProxyServer);

    /**
     * \brief Destructor
     *
     * Clean up an RtspClient object.
     */
    ~RtspClient();

private:
    /**
     * \brief Callback for constructing RtspMedia objects
     *
     * GST server factory calls this callback on "media-constructed" event
     */
    static void onConstructRtspMedia(
        GstRTSPMediaFactory*,
        GstRTSPMedia* gstRtspMedia,
        RtspClient* client);

    /** instance of the server this client connected to */
    RtspServer* m_server = nullptr;

    /** instance of the RTSP media this client instantiated */
    RtspMedia* m_rtspMedia = nullptr;

    /** pointer to internal instance of the gstreamer media object */
    GstRTSPClient* m_gstClient = nullptr;

    /** ID for gstreamer media callback created for this client */
    gulong m_mediaConstructedHandlerId = 0;

    /** ID for gstreamer media callback created for this client */
    gulong m_cliendClosedHandlerId = 0;
};

} // end of namespace

#endif
