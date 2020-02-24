// Open CV headers
#include <opencv2/imgproc/imgproc.hpp>  // cv::resize

// Project headers
#include <RtspProxyProcessor.hpp>

namespace rtsp_proxy_server {

#define DEBUG_PROXY_PROCESSOR 0

RtspProxyProcessor::RtspProxyProcessor(
    std::shared_ptr<const RtspProxyConfig> config)
    :
    m_buffer(config->getInputBufferSize()),
    m_outputSize(
        int(config->getOutputDimensions().width),
        int(config->getOutputDimensions().height))
{
    m_openCvReaders.resize(config->getInputPipelinesNum());
    m_lastFrame.resize(config->getInputPipelinesNum());

    // add one empty frame into the buffer so our consumer always has a "valid"
    // frame
    for (auto& f : m_lastFrame) {
        f = std::make_shared<cv::Mat>(2160, 3840, CV_8UC3, cv::Scalar(0));
    }

    // push one "good" frame so consumer has something valid to read
    // before we are ready
    m_buffer.push(m_lastFrame[0]);

    // Initialize a semaphore to get notified about incoming frames
    sem_init(&m_videoFrameReadySemaphore, 0, 0);

    // Open all configured GST pipelines
    for (size_t idx=0; idx < config->getInputPipelinesNum(); idx++ ) {
        m_openCvReaders[idx].reset(
            new OpenCvReader(
                config->getInputPipelines()[idx],
                config->getInputBufferSize(),
                &m_videoFrameReadySemaphore));
    }

    // start the reader's thread
    start();
}

RtspProxyProcessor::~RtspProxyProcessor()
{
    stop();
}

bool
RtspProxyProcessor::isConnected() const
{
    if (m_openCvReaders.size() == 0) {
        return false;
    }
    for (auto const& r : m_openCvReaders) {
        if (not r->isConnected()) {
            return false;
        }
    }
    return true;
}

void
RtspProxyProcessor::start()
{
    if (m_running) {
        fprintf(stderr, "WARNING: CvReader already running!\n");
        return;
    }
    m_running = true;
    m_thread =
        std::thread(&RtspProxyProcessor::rtspProxyProcessorThread, this);
}

void
RtspProxyProcessor::stop() {
    m_running = false;
    sem_post(&m_videoFrameReadySemaphore);
    try {
        if (m_thread.joinable()) {
            printf("Stopping RtspProxyProcessor thread...\n");
            m_thread.join();
            printf("RtspProxyProcessor thread stopped.\n");
        }
    }
    catch(std::runtime_error & e) {
        fprintf(
            stderr, "ERROR: Failed to JOIN RtspProxyProcessor thread. %s",
            e.what());
        fflush(stderr);
    }
}

void
RtspProxyProcessor::rtspProxyProcessorThread() {
    // wait for the cameras to be connected
    // or RTSP client to disconnect
    while (m_running && not isConnected()) {
        sem_wait(&m_videoFrameReadySemaphore);
    }

    if (m_running) {
        printf(
            "\nAll cameras connected. Starting RTSP Proxy processing...\n\n");
    }

    std::vector<CvMatPtr> currFrame;
    currFrame.resize(m_openCvReaders.size());

    while (m_running) {
        /*--------------------------------------------*/
        /*-- wait for a video frame      -------------*/
        /*--------------------------------------------*/
        sem_wait(&m_videoFrameReadySemaphore);

        #if DEBUG_PROXY_PROCESSOR
            auto start = std::chrono::steady_clock::now();
        #endif

        // allocate space for a new, processed, frame
        auto outputFrame = std::make_shared<cv::Mat>();

        //
        // load frames from all cameras. If a camera doesn't have a valid
        // frame - load the previous one saved for this camera
        //
        int w = 0;
        int h = 0;
        for (size_t i=0; i < m_openCvReaders.size(); i++) {
            currFrame[i] = m_openCvReaders[i]->getFrame();
            if (not currFrame[i]) {
                currFrame[i] = m_lastFrame[i];
            }
            w += currFrame[i]->cols;
            h = (h > currFrame[i]->rows) ? h : currFrame[i]->rows;
        }

        // Place all camera frames in one row

        // 1. Create canvas that fits all frames
        cv::Mat canvas(h, w, currFrame[0]->type());

        #if DEBUG_PROXY_PROCESSOR
            printf("Canvas: %d x %d\n", canvas.cols, canvas.rows);
        #endif

        w = 0;
        try {
            for (auto& frame : currFrame) {
                cv::Rect image(w, 0, frame->cols, frame->rows);

                frame->copyTo(canvas(image));

                w += frame->cols;
            }

            // adjust canvas size to our output size
            cv::resize(canvas, *outputFrame, m_outputSize);

        } catch(cv::Exception const& e) {
            fprintf(stderr, "OpenCV call Failed:\n\t%s\n", e.what());
            continue;
        }

        // send new processed frame to out consumer
        m_buffer.push(outputFrame);

        // save all last frames in case we cannot read fast enough from the
        // camera streams
        for (size_t i=0; i < currFrame.size(); i++) {
            m_lastFrame[i] = currFrame[i];
        }

        #if DEBUG_PROXY_PROCESSOR
            auto end = std::chrono::steady_clock::now();
            auto elapsed =
                    std::chrono::duration_cast<std::chrono::nanoseconds>(
                        end - start).count();
            printf("ProxyView processing took: %zu ns (%0.6lf s)\n",
                elapsed, double(elapsed)/1000./1000./1000.);
        #endif
    }
    m_running = false;
}

}
