#include "../include/render_config.hpp"
#include "../include/parallel_renderer.hpp"

RenderConfig::RenderConfig() {
    initDefaults();
}

void RenderConfig::initDefaults() {
    regionType = RegionType::COLUMN;
    regionSize = 8;
    numThreads = 4;
    queueType = QueueType::STD_QUEUE;
}
