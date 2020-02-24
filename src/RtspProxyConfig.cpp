#include <boost/algorithm/string/replace.hpp>

#include <RtspProxyConfig.hpp>

namespace rtsp_proxy_server {

RtspProxyConfig::RtspProxyConfig(std::string const& configFile)
{
    YAML::Node config;
    try {
         config = YAML::LoadFile(configFile);

    } catch(const YAML::Exception& ex) {
        throw std::runtime_error(
            "Failed to load file '" + configFile + "'. " + ex.msg);
    }

    m_inputBufferSize = config["input_ring_buffer_size"].as<uint>(1);
    if (m_inputBufferSize == 0) {
        throw std::runtime_error(
            "Input ring buffer size (input_ring_buffer_size) cannot be zero!");
    }

    m_inputRtspPort =
        config["input_rtsp_port_t"].as<ushort>(m_inputRtspPort);

    auto inputRtspHost = config["input_rtsp_host_t"].as<std::string>("");
    auto inputRtspUser = config["input_rtsp_user_t"].as<std::string>("");
    auto inputRtspPassword = config["input_rtsp_passwd_t"].as<std::string>("");
    auto inputRtspPath = config["input_rtsp_path_t"].as<std::string>("");
    auto inputRtspUrl = config["input_rtsp_url_t"].as<std::string>("");

    auto inputLocations =
        config["input_rtsp_locations_t"].as<std::vector<std::string>>(
            std::vector<std::string>());

    auto inputRtspPipelineIdxT =
        config["input_gst_rtsp_pipeline_idx_t"].as<std::string>("");

    m_inputPipelines =
        config["input_gst_rtsp_pipelines"].as<std::vector<std::string>>(
            m_inputPipelines);

    // The input information is loaded, now substitute all variables, if set.
    // The variables are in form of {VAR}

    auto SUB_TEMPLATES = [
        this,
        inputRtspUser,
        inputRtspPassword,
        inputRtspHost,
        inputRtspPath,
        &inputRtspUrl](std::string & dst)
    {
        boost::replace_all(dst, "{USER}", inputRtspUser);
        boost::replace_all(dst, "{PASSWD}", inputRtspPassword);
        boost::replace_all(dst, "{HOST}", inputRtspHost);
        boost::replace_all(dst, "{PORT}", std::to_string(m_inputRtspPort));
        boost::replace_all(dst, "{PATH}", inputRtspPath);
        boost::replace_all(dst, "{URL}", inputRtspUrl);
    };

    // process RtspUrl template first
    SUB_TEMPLATES(inputRtspUrl);

    // process all URLs used for gstreamer RTSP location
    for (auto& loc : inputLocations) {
        SUB_TEMPLATES(loc);
    }

    // now build actual pipeline for each camera we suppose to connect to
    for (size_t i=0; i < m_inputPipelines.size(); i++) {
        auto& pipe = m_inputPipelines[i];

        // the pipeline can have all single variables, as well as PIPELINE
        // variable that refers to a corresponding URL : {PIPELINE{URL}}
        SUB_TEMPLATES(pipe);

        if (pipe.find("{PIPELINE_IDX}") != std::string::npos) {
            if (inputLocations.size() <= i) {
                throw std::runtime_error(
                    "Invalid config. input_rtsp_locations_t is smaller than "
                    " input_gst_rtsp_pipelines. Required by {PIPELINE_IDX} in"
                    " element " + std::to_string(i));
            }
            std::string tmp = inputRtspPipelineIdxT;
            boost::replace_all(tmp, "{LOCATION}", inputLocations[i]);
            boost::replace_all(pipe, "{PIPELINE_IDX}", tmp);
        }
    }

    //
    // Load the output configuration
    //
    m_outputFps = config["output_fps"].as<uint>(m_outputFps);
    m_outputDimensions.width =
        config["output_width"].as<uint>(m_outputDimensions.width);
    m_outputDimensions.height =
        config["output_height"].as<uint>(m_outputDimensions.height);
    m_outputPath =
        config["output_path"].as<std::string>(m_outputPath);
    m_outputPipeline =
        config["output_gst_rtsp_pipeline"].as<std::string>(m_outputPipeline);

    boost::replace_all(
        m_outputPipeline,
        "{OUTPUT_WIDTH}",
        std::to_string(m_outputDimensions.width));

    boost::replace_all(
        m_outputPipeline,
        "{OUTPUT_HEIGHT}",
        std::to_string(m_outputDimensions.height));

    boost::replace_all(
        m_outputPipeline,
        "{OUTPUT_FPS}",
        std::to_string(m_outputFps));
}

}
