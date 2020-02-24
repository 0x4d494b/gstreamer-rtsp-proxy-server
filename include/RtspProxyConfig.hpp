#ifndef RTSP_PROXY_CONFIG_HPP
#define RTSP_PROXY_CONFIG_HPP

#include <string>
#include <vector>

#include <yaml-cpp/yaml.h>

namespace rtsp_proxy_server {

using CameraPipelines = std::vector<std::string>;

struct FrameDimensions {
    uint width = 0;
    uint height = 0;
};

class RtspProxyConfig {
public:
    /**
     * \brief Constructor for the configuration object
     *
     * \param[in] configFile path to yaml file containing the RTSP proxy
     *            server configuration
     */
    RtspProxyConfig(std::string const& configFile = "config/rtsp-proxy.yaml");

    /**
     * \brief Get gstreamer RTSP pipelines for connecting to input cameras.
     *        There should be 'NumCameras' cameras that will be used to
     *        compose an RTSP server view
     */
    CameraPipelines const& getInputPipelines() const
    {
        return m_inputPipelines;
    }

    /**
     * \brief Get number of configured input pipelines
     */
    size_t getInputPipelinesNum() const
    {
        return m_inputPipelines.size();
    }

    /**
     * \brief Get buffer size in number of frames for input cameras.
     *        This is a size of circular buffer for inputs. If frame is not
     *        consumed until a new frame arrives, it is dropped.
     */
    uint getInputBufferSize() const { return m_inputBufferSize; }

    /**
     * \brief Get gstreamer output pipeline for the RTSP proxy server
     */
    std::string const& getOutputPipeline() const { return m_outputPipeline; }

    /**
     * \brief Get gstreamer output path, or a mount point
     */
    std::string const& getOutputPath() const { return m_outputPath; }

    /**
     * \brief Get output stream FPS. This is the frequency at which our
     *        clients will request new frames, whether it's ready or not.
     */
    uint getOutputFps() const {return m_outputFps; }

    /**
     * \brief Get output frame dimensions
     */
    FrameDimensions const& getOutputDimensions() const {
        return m_outputDimensions;
    }

private:
    ushort m_inputRtspPort = 554;
    uint m_inputBufferSize = 3;

    CameraPipelines m_inputPipelines;

    uint m_outputFps = 0;
    FrameDimensions m_outputDimensions;

    std::string m_outputPath;
    std::string m_outputPipeline;
};

} // end of namespace
#endif
