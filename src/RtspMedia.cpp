#include <RtspMedia.hpp>

namespace rtsp_proxy_server {

RtspMedia::RtspMedia(
    GstRTSPMedia* rtspMedia,
    std::shared_ptr<RtspProxyConfig> config) :
    m_rtspProxyProcessor(config) // start proxy server processor
{
    m_frameDuration =
        GstClockTime(double(1. / double(config->getOutputFps())) * GST_SECOND);

    printf("Creating Media object for new RTSP client...\n");

    GstElement* appsrc = gst_rtsp_media_get_element(rtspMedia);
    GstElement* vsrc =
        gst_bin_get_by_name_recurse_up(GST_BIN(appsrc), "source");

    // attach a 'need-data' callback to the new RTSP media stream
    auto id = g_signal_connect(
        vsrc,
        "need-data",
        G_CALLBACK(&RtspMedia::onNeedData),
        static_cast<gpointer>(this));

    if (id <= 0) {
        throw std::runtime_error(
            "ERROR: failed to connect 'need-data' to 'source'");
    }
}

RtspMedia::~RtspMedia() {
}

GstFlowReturn
RtspMedia::onNeedData(
    GstElement* gstSrc,
    guint,
    RtspMedia* media)
{
    // get a new frame from the processor, if available
    auto frame = media->m_rtspProxyProcessor.getFrame();
    if (not frame) {
        // frame is not available - use the previous one
        frame = media->m_lastFrame;
    } else {
        media->m_lastFrame = frame;
    }

    if (frame->empty()) {
        fprintf(
            stderr,
            "got empty frame: framePtr=%p, frameNum=%zu.\n",
            frame.get(),
            media->m_frameNumber);
            fflush(stderr);
            //return GST_FLOW_NOT_LINKED;
    }

    auto dataSize = frame->total() * frame->elemSize();

    auto * buf = gst_buffer_new_and_alloc(dataSize);

    gst_buffer_fill(buf, 0, frame->data, dataSize);

    GST_BUFFER_OFFSET(buf) = media->m_frameNumber;
    GST_BUFFER_OFFSET_END(buf) = media->m_frameNumber;

    GST_BUFFER_DTS(buf) = media->m_frameTimestamp;
    GST_BUFFER_PTS(buf) = media->m_frameTimestamp;
    GST_BUFFER_DURATION(buf) =
        static_cast<GstClockTime>(media->m_frameDuration);

    GstFlowReturn ret = GST_FLOW_ERROR;
    g_signal_emit_by_name(gstSrc, "push-buffer", buf, &ret);
    gst_buffer_unref(buf);

    if (ret != GST_FLOW_OK) {
        fprintf(stderr, "\nERROR: g_signal_emit_by_name failed: %d\n", ret);
    }
    #if DEBUG
        else {
            printf(
                "pushed buffer: frame %zu, size: %zu, "
                "ts %zu ns, elapsed: %zu ns, duration %zu ns, res=%d\n",
                stream->m_frameNumber,
                dataSize,
                stream->m_frameTimestamp,
                elapsed,
                stream->m_frameDuration,
                //"durations %lf s"double(stream->m_duration) / GST_SECOND,
                ret);
            fflush(stdout);
        }
    #endif

    media->m_frameTimestamp += media->m_frameDuration;
    media->m_frameNumber ++;

    return ret;
}

}
