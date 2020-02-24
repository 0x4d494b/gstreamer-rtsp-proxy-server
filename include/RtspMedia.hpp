#ifndef RTSP_PROXY_RTSP_MEDIA_HPP
#define RTSP_PROXY_RTSP_MEDIA_HPP

#include <memory>
#include <chrono>

// gstreamer headers
#pragma clang diagnostic ignored "-Wold-style-cast"

// rtsp server headers
#include <gst/rtsp-server/rtsp-server.h>
#include <gst/rtsp-server/rtsp-media-factory.h>
#include <gst/rtsp-server/rtsp-media.h>

// project headers
#include <RtspProxyProcessor.hpp>
#include <RtspProxyConfig.hpp>
#include <OpenCvReader.hpp>

namespace rtsp_proxy_server {

using QuadFrame = std::vector<cv::Mat>;

class RtspMedia {

public:
    RtspMedia(
        GstRTSPMedia* rtspMedia,
        std::shared_ptr<RtspProxyConfig> config);

    ~RtspMedia();

private:
    static GstFlowReturn onNeedData(
        GstElement* gstSrc,
        guint size,
        RtspMedia* media);

private:
    /** The RTSP proxy processor that gives us ready to display video frames */
    RtspProxyProcessor m_rtspProxyProcessor;

    /** last received frame from the processor */
    CvMatPtr m_lastFrame;

    /** current RTSP frame number */
    guint64 m_frameNumber = 0;

    /** current RTSP frame timestamp from the begining of the stream */
    GstClockTime m_frameTimestamp = 0;

    /** RTSP frame duration in nanoseconds. Amount of time the client should
     *  display the frame
     */
    GstClockTime m_frameDuration = 0;
};

}

#endif
