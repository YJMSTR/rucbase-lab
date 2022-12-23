#include "lru_replacer.h"

LRUReplacer::LRUReplacer(size_t num_pages) { max_size_ = num_pages; }

LRUReplacer::~LRUReplacer() = default;

/**
 * @brief 使用 LRU 策略删除一个 victim frame，这个函数能得到 frame_id
 * @param[out] frame_id id of frame that was removed, nullptr if no victim was found
 * @return true if a victim frame was found, false otherwise
 */
bool LRUReplacer::Victim(frame_id_t *frame_id) {
    // C++17 std::scoped_lock
    // 它能够避免死锁发生，其构造函数能够自动进行上锁操作，析构函数会对互斥量进行解锁操作，保证线程安全。
    std::scoped_lock lock{latch_};

    // Todo:
    //  利用 lru_replacer 中的 LRUlist_,LRUHash_实现 LRU 策略
    //  选择合适的 frame 指定为淘汰页面，赋值给*frame_id
    if (LRUlist_.empty()) {
        frame_id = nullptr;
        //puts("frame_id == NULL");
        return false;
    }
    *frame_id = *LRUlist_.rbegin();
    //printf ("%d\n", *frame_id);
    LRUlist_.pop_back();
    LRUhash_.erase(*frame_id);
    return true;
}

/**
 * @brief 固定一个 frame, 表明它不应该成为 victim（即在 replacer 中移除该 frame_id）
 * @param frame_id the id of the frame to pin
 */
void LRUReplacer::Pin(frame_id_t frame_id) {
    std::scoped_lock lock{latch_};
    // Todo:
    // 固定指定 id 的 frame
    // 在数据结构中移除该 frame
    if (LRUhash_.count(frame_id)) {
        LRUlist_.erase(LRUhash_[frame_id]);
        LRUhash_.erase(frame_id);
    }
}

/**
 * 取消固定一个 frame, 表明它可以成为 victim（即将该 frame_id 添加到 replacer）
 * @param frame_id the id of the frame to unpin
 */
void LRUReplacer::Unpin(frame_id_t frame_id) {
    // Todo:
    //  支持并发锁
    //  选择一个 frame 取消固定
    std::scoped_lock lock{latch_};
    if (LRUhash_.count(frame_id)) return;
    LRUlist_.push_front(frame_id);
    std::list<frame_id_t>::iterator it = LRUlist_.begin();
    LRUhash_[frame_id] = it;
}

/** @return replacer 中能够 victim 的数量 */
size_t LRUReplacer::Size() {
    // Todo:
    // 改写 return size
    return LRUlist_.size();
}
