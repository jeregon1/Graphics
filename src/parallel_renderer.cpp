#include "../include/parallel_renderer.hpp"
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
std::vector<RenderTask> TaskGenerator::generateTasks(int width, int height, const ParallelConfig& config) {
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
ParallelRenderer::ParallelRenderer(const ParallelConfig& config) 
    : config_(config) {}

Image ParallelRenderer::renderPathTracing(const PinholeCamera& camera, const Scene& scene,
                                         unsigned samplesPerPixel) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    int width = camera.getWidth();
    int height = camera.getHeight();
    
    // Generate tasks
    std::vector<RenderTask> tasks = TaskGenerator::generateTasks(width, height, config_);
    
    // Initialize pixel buffer
    std::vector<RGB> pixels(width * height);
    
    // Reset task queue
    auto taskQueue = QueueFactory::createQueue(config_.queueType);
    
    // Add tasks to queue
    for (const auto& task : tasks) {
        taskQueue->push(task);
    }
    
    // Launch worker threads
    std::vector<std::thread> workers;
    std::atomic<int> completedTasks(0);
    
    for (int i = 0; i < config_.numThreads; ++i) {
        workers.emplace_back([&taskQueue, &camera, &scene, samplesPerPixel, &pixels, width, height, &completedTasks]() {
            RenderTask task(0, 0, 0, 0);
            
            while (taskQueue->pop(task)) {
                // Render the assigned region
                for (int y = task.startY; y < task.endY; ++y) {
                    float normalizedY = static_cast<float>(y) - (height / 2.0f);
                    
                    for (int x = task.startX; x < task.endX; ++x) {
                        float normalizedX = static_cast<float>(x) - (width / 2.0f);
                        
                        RGB pixelColor = camera.calculatePixelColorPathTracing(scene, normalizedX, normalizedY, samplesPerPixel);
                        pixels[y * width + x] = pixelColor;
                    }
                }
                
                completedTasks.fetch_add(1);
            }
        });
    }
    
    // Signal completion and wait for workers
    static_cast<StandardTaskQueue*>(taskQueue.get())->finish();
    
    for (auto& worker : workers) {
        worker.join();
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    // Update statistics
    lastStats_.renderTime = duration.count() / 1000.0;
    lastStats_.numTasks = static_cast<int>(tasks.size());
    lastStats_.numThreads = config_.numThreads;
    lastStats_.regionType = config_.regionType;
    lastStats_.regionSize = config_.regionSize;
    
    return Image(width, height, pixels);
}

Image ParallelRenderer::renderRayTracing(const PinholeCamera& camera, const Scene& scene,
                                        unsigned samplesPerPixel) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    int width = camera.getWidth();
    int height = camera.getHeight();
    
    // Generate tasks
    std::vector<RenderTask> tasks = TaskGenerator::generateTasks(width, height, config_);
    
    // Initialize pixel buffer
    std::vector<RGB> pixels(width * height);
    
    // Reset task queue
    auto taskQueue = QueueFactory::createQueue(config_.queueType);
    
    // Add tasks to queue
    for (const auto& task : tasks) {
        taskQueue->push(task);
    }
    
    // Launch worker threads
    std::vector<std::thread> workers;
    std::atomic<int> completedTasks(0);
    
    for (int i = 0; i < config_.numThreads; ++i) {
        workers.emplace_back([&taskQueue, &camera, &scene, samplesPerPixel, &pixels, width, height, &completedTasks]() {
            RenderTask task(0, 0, 0, 0);
            
            while (taskQueue->pop(task)) {
                // Render the assigned region
                for (int y = task.startY; y < task.endY; ++y) {
                    float normalizedY = static_cast<float>(y) - (height / 2.0f);
                    
                    for (int x = task.startX; x < task.endX; ++x) {
                        float normalizedX = static_cast<float>(x) - (width / 2.0f);
                        
                        RGB pixelColor = camera.calculatePixelColorRayTracing(scene, normalizedX, normalizedY, samplesPerPixel);
                        pixels[y * width + x] = pixelColor;
                    }
                }
                
                completedTasks.fetch_add(1);
            }
        });
    }
    
    // Signal completion and wait for workers
    static_cast<StandardTaskQueue*>(taskQueue.get())->finish();
    
    for (auto& worker : workers) {
        worker.join();
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    // Update statistics  
    lastStats_.renderTime = duration.count() / 1000.0;
    lastStats_.numTasks = static_cast<int>(tasks.size());
    lastStats_.numThreads = config_.numThreads;
    lastStats_.regionType = config_.regionType;
    lastStats_.regionSize = config_.regionSize;
    
    return Image(width, height, pixels);
}

Image ParallelRenderer::renderPhotonMapping(const PinholeCamera& camera, const Scene& scene,
                                           unsigned samplesPerPixel, MapaFotones mapa,
                                           unsigned kFotones, double radio, Kernel* kernel) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    int width = camera.getWidth();
    int height = camera.getHeight();
    
    // Generate tasks
    std::vector<RenderTask> tasks = TaskGenerator::generateTasks(width, height, config_);
    
    // Initialize pixel buffer
    std::vector<RGB> pixels(width * height);
    
    // Reset task queue
    auto taskQueue = QueueFactory::createQueue(config_.queueType);
    
    // Add tasks to queue
    for (const auto& task : tasks) {
        taskQueue->push(task);
    }
    
    // Launch worker threads
    std::vector<std::thread> workers;
    std::atomic<int> completedTasks(0);
    
    for (int i = 0; i < config_.numThreads; ++i) {
        workers.emplace_back([&taskQueue, &camera, &scene, samplesPerPixel, mapa, kFotones, radio, kernel, &pixels, width, height, &completedTasks]() {
            RenderTask task(0, 0, 0, 0);
            
            while (taskQueue->pop(task)) {
                // Render the assigned region
                for (int y = task.startY; y < task.endY; ++y) {
                    float normalizedY = static_cast<float>(y) - (height / 2.0f);
                    
                    for (int x = task.startX; x < task.endX; ++x) {
                        float normalizedX = static_cast<float>(x) - (width / 2.0f);
                        
                        RGB pixelColor = camera.calculatePixelColorPhotonMapping(scene, normalizedX, normalizedY, samplesPerPixel, mapa, kFotones, radio, false, kernel);
                        pixels[y * width + x] = pixelColor;
                    }
                }
                
                completedTasks.fetch_add(1);
            }
        });
    }
    
    // Signal completion and wait for workers
    static_cast<StandardTaskQueue*>(taskQueue.get())->finish();
    
    for (auto& worker : workers) {
        worker.join();
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    // Update statistics
    lastStats_.renderTime = duration.count() / 1000.0;
    lastStats_.numTasks = static_cast<int>(tasks.size());
    lastStats_.numThreads = config_.numThreads;
    lastStats_.regionType = config_.regionType;
    lastStats_.regionSize = config_.regionSize;
    
    return Image(width, height, pixels);
}

/**
 * RenderBenchmark Implementation
 */
void RenderBenchmark::benchmarkConfigurations(const PinholeCamera& camera, const Scene& scene,
                                             const std::vector<ParallelConfig>& configs,
                                             unsigned samplesPerPixel) {
    std::cout << "=== Parallel Rendering Benchmark ===\n";
    std::cout << "Configuration\t\tTime(s)\t\tTasks\t\tThreads\n";
    std::cout << "-------------------------------------------------------\n";
    
    for (const auto& config : configs) {
        ParallelRenderer renderer(config);
        
        auto startTime = std::chrono::high_resolution_clock::now();
        renderer.renderPathTracing(camera, scene, samplesPerPixel);
        auto endTime = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        auto stats = renderer.getLastRenderStats();
        
        std::string regionName;
        switch (config.regionType) {
            case RegionType::PIXEL: regionName = "PIXEL"; break;
            case RegionType::LINE: regionName = "LINE"; break;
            case RegionType::COLUMN: regionName = "COLUMN"; break;
            case RegionType::RECTANGLE: regionName = "RECTANGLE"; break;
        }
        
        std::cout << regionName << "(" << config.regionSize << ")\t\t"
                  << duration.count() / 1000.0 << "\t\t"
                  << stats.numTasks << "\t\t"
                  << stats.numThreads << "\n";
    }
}

ParallelConfig RenderBenchmark::findOptimalConfig(const PinholeCamera& camera, const Scene& scene,
                                                 unsigned samplesPerPixel) {
    std::vector<ParallelConfig> configs = {
        ParallelConfig(RegionType::RECTANGLE, 16),
        ParallelConfig(RegionType::RECTANGLE, 32),
        ParallelConfig(RegionType::RECTANGLE, 64),
        ParallelConfig(RegionType::LINE, 1),
        ParallelConfig(RegionType::LINE, 4),
        ParallelConfig(RegionType::LINE, 8),
        ParallelConfig(RegionType::COLUMN, 1),
        ParallelConfig(RegionType::COLUMN, 4),
        ParallelConfig(RegionType::COLUMN, 8)
    };
    
    ParallelConfig bestConfig = configs[0];
    double bestTime = std::numeric_limits<double>::max();
    
    for (const auto& config : configs) {
        ParallelRenderer renderer(config);
        
        auto startTime = std::chrono::high_resolution_clock::now();
        renderer.renderPathTracing(camera, scene, samplesPerPixel);
        auto endTime = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        double time = duration.count() / 1000.0;
        
        if (time < bestTime) {
            bestTime = time;
            bestConfig = config;
        }
    }
    
    return bestConfig;
}
