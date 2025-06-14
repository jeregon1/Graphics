#pragma once

#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <functional>
#include <vector>
#include <memory>

#include "object3D.hpp"
#include "Image.hpp"
#include "render_config.hpp"

// Forward declarations
class Scene;
class PinholeCamera;

/**
 * Configuration for parallelization strategies
 */
enum class RegionType {
    PIXEL,      // Individual pixels
    LINE,       // Horizontal lines  
    COLUMN,     // Vertical columns
    RECTANGLE   // Rectangular blocks
};

enum class QueueType {
    STD_QUEUE,          // Standard queue with mutex
    LOCK_FREE_QUEUE,    // Future: lock-free implementation
    WORK_STEALING       // Future: work-stealing queue
};

/**
 * Represents a work unit for rendering
 */
struct RenderTask {
    int startX, startY;     // Starting coordinates
    int endX, endY;         // Ending coordinates (exclusive)
    int taskId;             // For debugging/profiling
    
    RenderTask(int sx, int sy, int ex, int ey, int id = 0)
        : startX(sx), startY(sy), endX(ex), endY(ey), taskId(id) {}
};

/**
 * Thread-safe task queue interface
 */
class TaskQueue {
public:
    virtual ~TaskQueue() = default;
    virtual void push(const RenderTask& task) = 0;
    virtual bool pop(RenderTask& task) = 0;
    virtual bool empty() const = 0;
    virtual size_t size() const = 0;
};

/**
 * Standard mutex-based task queue
 */
class StandardTaskQueue : public TaskQueue {
private:
    mutable std::mutex mutex_;
    std::queue<RenderTask> queue_;
    std::condition_variable condition_;
    std::atomic<bool> finished_{false};

public:
    void push(const RenderTask& task) override;
    bool pop(RenderTask& task) override;
    bool empty() const override;
    size_t size() const override;
    void finish();
};

/**
 * Task generator - creates tasks based on configuration
 */
class TaskGenerator {
public:
    static std::vector<RenderTask> generateTasks(
        int width, int height, const RenderConfig& config);

private:
    static std::vector<RenderTask> generatePixelTasks(int width, int height);
    static std::vector<RenderTask> generateLineTasks(int width, int height, int lineSize);
    static std::vector<RenderTask> generateColumnTasks(int width, int height, int columnSize);
    static std::vector<RenderTask> generateRectangleTasks(int width, int height, int blockSize);
};

/**
 * Parallel renderer - coordinates the rendering process
 */
class ParallelRenderer {
private:
    RenderConfig config_;
    std::unique_ptr<TaskQueue> taskQueue_;
    
    // Worker thread function
    void workerThread(const PinholeCamera& camera, const Scene& scene, 
                     unsigned samplesPerPixel, std::vector<RGB>& pixels,
                     int width, int height, std::atomic<int>& completedTasks);

public:
    explicit ParallelRenderer(const RenderConfig& config = RenderConfig());

    // Unified render method picks algorithm from config.algorithm
    Image render(const PinholeCamera& camera, const Scene& scene,
                 unsigned samplesPerPixel, const RenderConfig& cfg);

    // Configuration
    void setConfig(const RenderConfig& config) { config_ = config; }
    const RenderConfig& getConfig() const { return config_; }
    
    // Statistics
    struct RenderStats {
        double renderTime;
        int numTasks;
        int numThreads;
        RegionType regionType;
        int regionSize;
    };
    
    RenderStats getLastRenderStats() const { return lastStats_; }

private:
    mutable RenderStats lastStats_;
    // Generic parallel runner
    Image runParallel(const PinholeCamera& camera, const Scene& scene,
                     unsigned samplesPerPixel,
                     const RenderConfig& cfg) const;
};

/**
 * Factory for creating different queue types
 */
class QueueFactory {
public:
    static std::unique_ptr<TaskQueue> createQueue(QueueType type);
};

/**
 * Performance benchmarking utilities
 */
class RenderBenchmark {
public:
    static void benchmarkConfigurations(const PinholeCamera& camera, const Scene& scene,
                                       const std::vector<RenderConfig>& configs,
                                       unsigned samplesPerPixel = 4);
    static RenderConfig findOptimalConfig(const PinholeCamera& camera, const Scene& scene,
                                         unsigned samplesPerPixel = 4);
};
