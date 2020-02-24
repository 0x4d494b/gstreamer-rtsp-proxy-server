// Project headers
#include <OpenCvReader.hpp>

namespace rtsp_proxy_server {

#define DEBUG_OPEN_CV_READER 0

OpenCvReader::OpenCvReader(
    std::string const& gstPipeline,
    uint bufferSize,
    sem_t *sem)
    :
    m_gstPipeline(gstPipeline),
    m_videoFrameReadySemaphore(sem),
    m_buffer(bufferSize)
{
    assert(m_videoFrameReadySemaphore != nullptr);

    // start the reader's thread
    start();
}

OpenCvReader::~OpenCvReader()
{
    stop();
    closeCam();
}

void
OpenCvReader::start()
{
    if (m_running) {
        fprintf(stderr, "WARNING: CvReader already running!\n");
        return;
    }
    m_running = true;
    m_cvReaderThread = std::thread(&OpenCvReader::cvReaderThread, this);
}

void
OpenCvReader::stop() {
    m_running = false;
    try {
        if (m_cvReaderThread.joinable()) {
            printf("Stopping OpenCvReader thread...\n");
            m_cvReaderThread.join();
            printf("OpenCvReader thread stopped.\n");
        }
    }
    catch(std::runtime_error & e) {
        fprintf(
            stderr, "ERROR: Failed to JOIN cv reader thread. %s",
            e.what());
        fflush(stderr);
    }
}

bool
OpenCvReader::openCam()
{
    bool success = true;

    printf("\nConnecting to GST pipeline:\n\t'%s'...\n", m_gstPipeline.c_str());

    m_videoCapture = cv::VideoCapture(m_gstPipeline, cv::CAP_GSTREAMER);
    if (not m_videoCapture.isOpened()) {
        fprintf(
            stderr,
            "\nERROR: Unable to open pipeline:\n\t'%s'\n",
            m_gstPipeline.c_str());
        success = false;
    } else {
        printf("\nOpened VideoCapture for:\n\t'%s'\n\n", m_gstPipeline.c_str());
        cv::waitKey(1);
    }
    return success;
}

void
OpenCvReader::closeCam()
{
    if (m_videoCapture.isOpened()) {
        printf("\nReleasing VideoCapture:\n\t%s\n", m_gstPipeline.c_str());
        m_videoCapture.release();
    }
}

CvMatPtr
OpenCvReader::getFrame()
{
    CvMatPtr currFrame;
    m_buffer.pop(currFrame);
    return currFrame;
}

void
OpenCvReader::cvReaderThread()
{
    // connect to the input camera
    openCam();

    if (not isConnected()) {
        fprintf(
            stderr,
            "ERROR: pipeline %s\n"
            " Attempted to start VideoCapture thread when not connected\n",
            m_gstPipeline.c_str());
        m_running = false;
        return;
    }

    while (m_running) {
        /*--------------------------------------------*/
        /*-- Read in source video stream -------------*/
        /*--------------------------------------------*/
        auto f = std::make_shared<cv::Mat>();

        bool success = m_videoCapture.read(*f); // read a new video frame
        if (!success || f->empty()) {
            fprintf(
                stderr,
                "Pipeline '%s'\n\tFailed to read a frame\n",
                m_gstPipeline.c_str());
            fflush(stderr);
            continue;
        }
        #if DEBUG_OPEN_CV_READER
            else {
                printf("cv: got frame from %s\n", m_gstPipeline.c_str());
                fflush(stdout);
            }
        #endif

        m_buffer.push(f);
        sem_post(m_videoFrameReadySemaphore); // notify the consumer

        //cv::waitKey(1);
    }

    m_running = false;

    closeCam();
}

}
