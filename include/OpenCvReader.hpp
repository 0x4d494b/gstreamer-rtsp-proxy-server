#ifndef OPEN_CV_READER_HPP
#define OPEN_CV_READER_HPP

// System headers
#include <semaphore.h>

// STL headers
#include <atomic>
#include <thread>

// Boost headers
#include <boost/lockfree/spsc_queue.hpp>

// Open CV headers
#include <opencv2/core/core.hpp>        // cv::Mat
#include <opencv2/highgui/highgui.hpp>  // cv::VideoCapture

namespace rtsp_proxy_server {

using CvMatPtr = std::shared_ptr<cv::Mat>;
using FrameBuffer = boost::lockfree::spsc_queue<CvMatPtr>;

class OpenCvReader {
public:
    /**
     * \brief Constructor for OpenCvReader
     *
     * \param[in] gstPipeline GSTREAMER pipeline to an RTSP server for low
     *            latency RTSP stream open using OpenCV API
     * \param[in] bufferSize size of a ring buffer for holding video frames
     * \param[in] sem semaphore to signal a consumer that a video frame is ready
     */
    OpenCvReader(
        std::string const& gstPipeline,
        uint bufferSize,
        sem_t* sem);

    /**
     * \brief Destructor
     *
     * Clean up the object.
     */
    ~OpenCvReader();

    bool isConnected() { return m_videoCapture.isOpened(); }

    CvMatPtr getFrame();

    void start();

    void stop();

private:
    bool openCam();

    void closeCam();

    void cvReaderThread();

private:
    /** GST Pipeline used to create the capture (for reference) */
    std::string m_gstPipeline;

    /** OpenCV video capture device connected to a remote RTSP server */
    cv::VideoCapture m_videoCapture;

    /** A consumer created semaphore to signal that a video frame is ready */
    sem_t* m_videoFrameReadySemaphore = nullptr;

    /** Circular buffer to store OpenCV video frames */
    FrameBuffer m_buffer;

    /** Thread to read RTSP frames */
    std::thread m_cvReaderThread;

    /** Indicates if openCV thread is running */
    std::atomic<bool> m_running = {false};
};

} // end of namespace

#endif
