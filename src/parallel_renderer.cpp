#include "../include/parallel_renderer.hpp"
#include "../include/pinholeCamera.hpp"
#include "../include/rendering_strategy.hpp"
#include <chrono>
#include <iostream>
#include <algorithm>

/**
 * StandardTaskQueue Implementation
 */
void StandardTaskQueue::push(const RenderTask& task) {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(task);
    condition_.notify_one();
}

bool StandardTaskQueue::pop(RenderTask& task) {
    std::unique_lock<std::mutex> lock(mutex_);
    condition_.wait(lock, [this] { return !queue_.empty() || finished_.load(); });
    
    if (queue_.empty()) {
        return false;
    }
    
    task = queue_.front();
    queue_.pop();
    return true;
}

bool StandardTaskQueue::empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
}

size_t StandardTaskQueue::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
}

void StandardTaskQueue::finish() {
    finished_.store(true);
    condition_.notify_all();
}

/**
 * TaskGenerator Implementation
 */
std::vector<RenderTask> TaskGenerator::generateTasks(int width, int height, const RenderConfig& config) {
    switch (config.regionType) {
        case RegionType::PIXEL:
            return generatePixelTasks(width, height);
        case RegionType::LINE:
            return generateLineTasks(width, height, config.regionSize);
        case RegionType::COLUMN:
            return generateColumnTasks(width, height, config.regionSize);
        case RegionType::RECTANGLE:
            return generateRectangleTasks(width, height, config.regionSize);
        default:
            return generateRectangleTasks(width, height, config.regionSize);
    }
}

std::vector<RenderTask> TaskGenerator::generatePixelTasks(int width, int height) {
    std::vector<RenderTask> tasks;
    tasks.reserve(width * height);
    
    int taskId = 0;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            tasks.emplace_back(x, y, x + 1, y + 1, taskId++);
        }
    }
    return tasks;
}

std::vector<RenderTask> TaskGenerator::generateLineTasks(int width, int height, int lineSize) {
    std::vector<RenderTask> tasks;
    
    int taskId = 0;
    for (int y = 0; y < height; y += lineSize) {
        int endY = std::min(y + lineSize, height);
        tasks.emplace_back(0, y, width, endY, taskId++);
    }
    return tasks;
}

std::vector<RenderTask> TaskGenerator::generateColumnTasks(int width, int height, int columnSize) {
    std::vector<RenderTask> tasks;
    
    int taskId = 0;
    for (int x = 0; x < width; x += columnSize) {
        int endX = std::min(x + columnSize, width);
        tasks.emplace_back(x, 0, endX, height, taskId++);
    }
    return tasks;
}

std::vector<RenderTask> TaskGenerator::generateRectangleTasks(int width, int height, int blockSize) {
    std::vector<RenderTask> tasks;
    
    int taskId = 0;
    for (int y = 0; y < height; y += blockSize) {
        for (int x = 0; x < width; x += blockSize) {
            int endX = std::min(x + blockSize, width);
            int endY = std::min(y + blockSize, height);
            tasks.emplace_back(x, y, endX, endY, taskId++);
        }
    }
    return tasks;
}

/**
 * QueueFactory Implementation
 */
std::unique_ptr<TaskQueue> QueueFactory::createQueue(QueueType type) {
    switch (type) {
        case QueueType::STD_QUEUE:
            return std::make_unique<StandardTaskQueue>();
        case QueueType::LOCK_FREE_QUEUE:
            // Future implementation
            std::cerr << "Lock-free queue not implemented yet, using standard queue\n";
            return std::make_unique<StandardTaskQueue>();
        case QueueType::WORK_STEALING:
            // Future implementation
            std::cerr << "Work-stealing queue not implemented yet, using standard queue\n";
            return std::make_unique<StandardTaskQueue>();
        default:
            return std::make_unique<StandardTaskQueue>();
    }
}

/**
 * ParallelRenderer Implementation
 */
ParallelRenderer::ParallelRenderer(const RenderConfig& config)
    : config_(config) {}

// Unified parallel render entry point
Image ParallelRenderer::render(const PinholeCamera& camera,
                               const Scene& scene,
                               unsigned samplesPerPixel,
                               const RenderConfig& cfg) {
    config_ = cfg;
    return runParallel(camera, scene, samplesPerPixel, cfg);
}

// Updated signature: drop explicit algorithm parameter
Image ParallelRenderer::runParallel(
    const PinholeCamera& camera,
    const Scene& scene,
    unsigned samplesPerPixel,
    const RenderConfig& cfg
) const {
    auto startTime = std::chrono::high_resolution_clock::now();

    int width = camera.getWidth();
    int height = camera.getHeight();

    auto tasks = TaskGenerator::generateTasks(width, height, cfg);

    std::vector<RGB> pixels(width * height);
    auto taskQueue = QueueFactory::createQueue(cfg.queueType);

    for (auto& t : tasks) {
        taskQueue->push(t);
    }

    // Pick strategy from cfg.algorithm
    auto strategy = StrategyFactory::createStrategy(cfg.algorithm);

    std::vector<std::thread> workers;

    for (int i = 0; i < cfg.numThreads; ++i) {
        workers.emplace_back([&, width, height]() {
            RenderTask task(0, 0, 0, 0);

            while (taskQueue->pop(task)) {
                for (int y = task.startY; y < task.endY; ++y) {
                    float ny = float(y) - (height / 2.0f);

                    for (int x = task.startX; x < task.endX; ++x) {
                        float nx = float(x) - (width / 2.0f);

                        pixels[y * width + x] = strategy->calculatePixelColor(
                            camera, scene, nx, ny, samplesPerPixel, cfg
                        );
                    }
                }
            }
        });
    }

    static_cast<StandardTaskQueue*>(taskQueue.get())->finish();

    for (auto& w : workers) {
        w.join();
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    lastStats_ = {
        dur.count() / 1000.0,
        int(tasks.size()),
        cfg.numThreads,
        cfg.regionType,
        cfg.regionSize
    };

    return Image(width, height, pixels);
}

/**
 * RenderBenchmark Implementation
 */
void RenderBenchmark::benchmarkConfigurations(const PinholeCamera& camera, const Scene& scene,
                                             const std::vector<RenderConfig>& configs,
                                             unsigned samplesPerPixel) {
    std::cout << "=== Parallel Rendering Benchmark ===\n";
    std::cout << "Configuration\t\tTime(s)\t\tTasks\t\tThreads\n";
    std::cout << "-------------------------------------------------------\n";
    for (const auto& cfg : configs) {
        ParallelRenderer renderer(cfg);
        auto startTime = std::chrono::high_resolution_clock::now();
        renderer.render(camera, scene, samplesPerPixel, cfg);
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        auto stats = renderer.getLastRenderStats();
        std::string regionName;
        switch (cfg.regionType) {
            case RegionType::PIXEL: regionName = "PIXEL"; break;
            case RegionType::LINE: regionName = "LINE"; break;
            case RegionType::COLUMN: regionName = "COLUMN"; break;
            case RegionType::RECTANGLE: regionName = "RECTANGLE"; break;
        }
        std::cout << regionName << "(" << cfg.regionSize << ")\t\t"
                  << duration.count() / 1000.0 << "\t\t"
                  << stats.numTasks << "\t\t"
                  << stats.numThreads << "\n";
    }
}

RenderConfig RenderBenchmark::findOptimalConfig(const PinholeCamera& camera, const Scene& scene,
                                               unsigned samplesPerPixel) {
    std::vector<RenderConfig> configs = {
        [](){ RenderConfig c; c.regionType=RegionType::RECTANGLE; c.regionSize=16; c.numThreads=4; return c; }(),
        [](){ RenderConfig c; c.regionType=RegionType::RECTANGLE; c.regionSize=32; c.numThreads=4; return c; }(),
        [](){ RenderConfig c; c.regionType=RegionType::RECTANGLE; c.regionSize=64; c.numThreads=4; return c; }(),
        [](){ RenderConfig c; c.regionType=RegionType::LINE; c.regionSize=1; c.numThreads=4; return c; }(),
        [](){ RenderConfig c; c.regionType=RegionType::LINE; c.regionSize=4; c.numThreads=4; return c; }(),
        [](){ RenderConfig c; c.regionType=RegionType::LINE; c.regionSize=8; c.numThreads=4; return c; }(),
        [](){ RenderConfig c; c.regionType=RegionType::COLUMN; c.regionSize=1; c.numThreads=4; return c; }(),
        [](){ RenderConfig c; c.regionType=RegionType::COLUMN; c.regionSize=4; c.numThreads=4; return c; }(),
        [](){ RenderConfig c; c.regionType=RegionType::COLUMN; c.regionSize=8; c.numThreads=4; return c; }(),
    };
    RenderConfig bestConfig = configs[0];
    double bestTime = std::numeric_limits<double>::max();
    for (const auto& cfg : configs) {
        ParallelRenderer renderer(cfg);
        auto startTime = std::chrono::high_resolution_clock::now();
        renderer.render(camera, scene, samplesPerPixel, cfg);
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        double time = duration.count() / 1000.0;
        if (time < bestTime) {
            bestTime = time;
            bestConfig = cfg;
        }
    }
    return bestConfig;
}
