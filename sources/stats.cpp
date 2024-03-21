#include "otpch.h"
#include "stats.h"
#include "tools.h"
#include "dispatcher.h"
#include <fstream>

uint32_t Stats::SLOW_EXECUTION_TIME = 10000000; // 10 ms
uint32_t Stats::VERY_SLOW_EXECUTION_TIME = 50000000; // 50 ms
int64_t Stats::DUMP_INTERVAL = 30000; // 5 min

AutoStatRecursive* AutoStatRecursive::activeStat = nullptr;


void Stats::threadMain() {
    std::unique_lock<std::mutex> taskLockUnique(statsLock, std::defer_lock);
    bool last_iteration = false;
    lua.lastDump = sql.lastDump = special.lastDump = OTSYS_TIME();
    playersOnline = 0;
    for(auto& dispatcher : dispatchers) {
        dispatcher.waitTime = 0;
        dispatcher.lastDump = OTSYS_TIME();
    }
    while(true) {
        taskLockUnique.lock();
        std::vector<std::forward_list < Task * >> tasks;
        for (auto &dispatcher : dispatchers) {
            tasks.push_back(std::move(dispatcher.queue));
            dispatcher.queue.clear();
        }
        std::forward_list < Stat * > lua_stats(std::move(lua.queue));
        lua.queue.clear();
        std::forward_list < Stat * > sql_stats(std::move(sql.queue));
        sql.queue.clear();
        std::forward_list < Stat * > special_stats(std::move(special.queue));
        special.queue.clear();
        taskLockUnique.unlock();

        parseDispatchersQueue(tasks);
        parseLuaQueue(lua_stats);
        parseSqlQueue(sql_stats);
        parseSpecialQueue(special_stats);

        int threadId = 0;
        for (auto &dispatcher : dispatchers) {
            if (dispatcher.lastDump + DUMP_INTERVAL < OTSYS_TIME() || last_iteration) {
                std::stringstream ss;
                float execution_time = 0;
                for (auto &it : dispatcher.stats)
                    execution_time += it.second.executionTime;
                ss << "Thread: " << ++threadId << " Cpu usage: " << (execution_time / 10000.) / ((float) DUMP_INTERVAL) << "%" <<
                   " Idle: " << (dispatcher.waitTime / 10000.) / ((float) DUMP_INTERVAL) << "%" <<
                   " Other: " << 100. - (((execution_time + dispatcher.waitTime) / 10000.) / ((float) DUMP_INTERVAL)) << "%";
                ss << " Players online: " << playersOnline << "\n";
                if(dispatcher.waitTime > 0)
                    writeStats("dispatcher.log", dispatcher.stats, ss.str());
                dispatcher.stats.clear();
                dispatcher.waitTime = 0;
                dispatcher.lastDump = OTSYS_TIME();
            }
        }
        if(lua.lastDump + DUMP_INTERVAL < OTSYS_TIME() || last_iteration) {
            writeStats("lua.log", lua.stats);
            lua.stats.clear();
            lua.lastDump = OTSYS_TIME();
        }
        if(sql.lastDump + DUMP_INTERVAL < OTSYS_TIME() || last_iteration) {
            writeStats("sql.log", sql.stats);
            sql.stats.clear();
            sql.lastDump = OTSYS_TIME();
        }
        if(special.lastDump + DUMP_INTERVAL < OTSYS_TIME() || last_iteration) {
            writeStats("special.log", special.stats);
            special.stats.clear();
            special.lastDump = OTSYS_TIME();
        }

        if(last_iteration)
            break;
        if(getState() == THREAD_STATE_TERMINATED) {
            last_iteration = true;
            continue;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void Stats::addDispatcherTask(int index, Task* task) {
    std::lock_guard<std::mutex> lockClass(statsLock);
    dispatchers[index].queue.push_front(task);
}

void Stats::addLuaStats(Stat* stats) {
    std::lock_guard<std::mutex> lockClass(statsLock);
    lua.queue.push_front(stats);
}

void Stats::addSqlStats(Stat* stats) {
    std::lock_guard<std::mutex> lockClass(statsLock);
    sql.queue.push_front(stats);
}

void Stats::addSpecialStats(Stat* stats) {
    std::lock_guard<std::mutex> lockClass(statsLock);
    special.queue.push_front(stats);
}

void Stats::parseDispatchersQueue(std::vector<std::forward_list < Task * >> queues) {
    int i = 0;
    for(auto& dispatcher : dispatchers) {
        for(Task* task : queues[i++]) {
            auto it = dispatcher.stats.emplace(task->description, statsData(0, 0, task->extraDescription)).first;
            it->second.calls += 1;
            it->second.executionTime += task->executionTime;
            if(task->executionTime > VERY_SLOW_EXECUTION_TIME) {
                writeSlowInfo("dispatcher_very_slow.log", task->executionTime, task->description, task->extraDescription);
            } else if(task->executionTime > SLOW_EXECUTION_TIME) {
                writeSlowInfo("dispatcher_slow.log", task->executionTime, task->description, task->extraDescription);
            }
            delete task;
        }
    }
}

void Stats::parseLuaQueue(std::forward_list <Stat*>& queue) {
    for(Stat* stats : queue) {
        auto it = lua.stats.emplace(stats->description, statsData(0, 0, stats->extraDescription)).first;
        it->second.calls += 1;
        it->second.executionTime += stats->executionTime;

        if(stats->executionTime > VERY_SLOW_EXECUTION_TIME) {
            writeSlowInfo("lua_very_slow.log", stats->executionTime, stats->description, stats->extraDescription);
        } else if(stats->executionTime > SLOW_EXECUTION_TIME) {
            writeSlowInfo("lua_slow.log", stats->executionTime, stats->description, stats->extraDescription);
        }
        delete stats;
    }
}

void Stats::parseSqlQueue(std::forward_list <Stat*>& queue) {
    for(Stat* stats : queue) {
        auto it = sql.stats.emplace(stats->description, statsData(0, 0, stats->extraDescription)).first;
        it->second.calls += 1;
        it->second.executionTime += stats->executionTime;

        if(stats->executionTime > VERY_SLOW_EXECUTION_TIME) {
            writeSlowInfo("sql_very_slow.log", stats->executionTime, stats->description, stats->extraDescription);
        } else if(stats->executionTime > SLOW_EXECUTION_TIME) {
            writeSlowInfo("sql_slow.log", stats->executionTime, stats->description, stats->extraDescription);
        }
        delete stats;
    }
}

void Stats::parseSpecialQueue(std::forward_list <Stat*>& queue) {
    for(Stat* stats : queue) {
        auto it = special.stats.emplace(stats->description, statsData(0, 0, stats->extraDescription)).first;
        it->second.calls += 1;
        it->second.executionTime += stats->executionTime;

        if(stats->executionTime > VERY_SLOW_EXECUTION_TIME) {
            writeSlowInfo("special_very_slow.log", stats->executionTime, stats->description, stats->extraDescription);
        } else if(stats->executionTime > SLOW_EXECUTION_TIME) {
            writeSlowInfo("special_slow.log", stats->executionTime, stats->description, stats->extraDescription);
        }
        delete stats;
    }
}

void Stats::writeSlowInfo(const std::string& file, uint64_t executionTime, const std::string& description, const std::string& extraDescription) {
    std::ofstream out(std::string("logs/") + file, std::ofstream::out | std::ofstream::app);
    if (!out.is_open()) {
        std::clog << "Can't open " << std::string("logs/") + file << " (check if directory exists)" << std::endl;
        return;
    }
    out << "[" << formatDate(time(NULL)) << "] Execution time: " << (executionTime / 1000000) << " ms - " << description << " - " << extraDescription << "\n";
    out.flush();
    out.close();
}

void Stats::writeStats(const std::string& file, const statsMap& stats, const std::string& extraInfo) {
    std::ofstream out(std::string("logs/") + file, std::ofstream::out | std::ofstream::app);
    if (!out.is_open()) {
        std::clog << "Can't open " << std::string("logs/") + file << " (check if directory exists)" << std::endl;
        return;
    }
    if(stats.empty()) {
        out.close();
        return;
    }
    out << "[" << formatDate(time(NULL)) << "]\n";

    std::vector<std::pair<std::string, statsData>> pairs;
    for (auto& it : stats)
        pairs.push_back(it);

    sort(pairs.begin(), pairs.end(), [=](const std::pair<std::string, statsData>& a, const std::pair<std::string, statsData>& b) {
        return a.second.executionTime > b.second.executionTime;
    });

    out << extraInfo;
    float total_time = 0;
    out << std::setw(10) << "Time (ms)" << std::setw(10) << "Calls"
        << std::setw(15) << "Rel usage " << "%" << std::setw(15) << "Real usage " << "%" << " " << "Description" << "\n";
    for(auto& it : pairs)
        total_time += it.second.executionTime;
    for(auto& it : pairs) {
        float percent = 100 * (float)it.second.executionTime / total_time;
        float realPercent = (float)it.second.executionTime / ((float)DUMP_INTERVAL * 10000.);
        if(percent > 0.1)
            out << std::setw(10) << it.second.executionTime / 1000000 << std::setw(10) << it.second.calls
                << std::setw(15) << std::setprecision(5) << std::fixed << percent << "%" << std::setw(15) << std::setprecision(5) << std::fixed << realPercent << "%" << " " << it.first << "\n";
    }
    out << "\n";
    out.flush();
    out.close();
}
