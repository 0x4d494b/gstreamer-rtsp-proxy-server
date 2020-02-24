#ifndef RTSP_PROXY_PROCESSOR_HPP
#define RTSP_PROXY_PROCESSOR_HPP

// System headers
#include <semaphore.h>

// STL headers
#include <atomic>
#include <thread>

// Boost headers
#include <boost/lockfree/spsc_queue.hpp>

// Project headers
#include <RtspProxyConfig.hpp>
#include <OpenCvReader.hpp>

namespace rtsp_proxy_server {

class RtspProxyProcessor {
public:
    /**
     * \brief Constructor
     */
    RtspProxyProcessor(std::shared_ptr<const RtspProxyConfig>);

    /**
     * \brief Destructor
     *
     * Clean up the object.
     */
    ~RtspProxyProcessor();

    bool isConnected() const;

    CvMatPtr getFrame() {
        CvMatPtr ptr;
        m_buffer.pop(ptr);
        return ptr;
    }

private:
    /**
     * \brief Start RTSP proxy processor
     */
    void start();

    /**
     * \brief Stop RTSP proxy processor
     */
    void stop();

    /**
     * \brief Thread reading all cameras inputs and processing them into
     *        a single frame for our RTSP server
     */
    void rtspProxyProcessorThread();

private:
    /** readers for all camera inputs */
    std::vector<std::unique_ptr<OpenCvReader>> m_openCvReaders;

    /** last received frames for all cameras */
    std::vector<CvMatPtr> m_lastFrame;

    /** A consumer created semaphore to signal that a video frame is ready */
    sem_t m_videoFrameReadySemaphore;

    /** Circular buffer to store processed OpenCV video frames */
    FrameBuffer m_buffer;

    /** Dimensions of an output frame */
    cv::Size m_outputSize;

    /** Thread to read and process all input RTSP frames */
    std::thread m_thread;

    /** Indicates if openCV thread is running */
    std::atomic<bool> m_running = {false};
};

} // end of namespace

#endif
