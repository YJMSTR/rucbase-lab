#include "rm_scan.h"

#include "rm_file_handle.h"

/**
 * @brief 初始化 file_handle 和 rid
 *
 * @param file_handle
 */
RmScan::RmScan(const RmFileHandle *file_handle) : file_handle_(file_handle) {
    // Todo:
    // 初始化 file_handle 和 rid（指向第一个存放了记录的位置）
    rid_.page_no = 0;
    
    do {
        RmPageHandle page_handle = file_handle_->fetch_page_handle(rid_.page_no);
        int fb = Bitmap::first_bit(true, page_handle.slots, BITMAP_WIDTH);
        if (fb != BITMAP_WIDTH) {
            rid_.slot_no = fb;
            return;
        } else {
            if (rid_.page_no == file_handle_->file_hdr_.num_pages) {
                rid_.page_no = rid_.slot_no = -1;
                return;
            }
            rid_.page_no++;
        }
    } while (1);
}

/**
 * @brief 找到文件中下一个存放了记录的位置
 */
void RmScan::next() {
    // Todo:
    // 找到文件中下一个存放了记录的非空闲位置，用 rid_来指向这个位置
    RmPageHandle page_handle = file_handle_->fetch_page_handle(rid_.page_no);
    do {
        int nb = Bitmap::next_bit(true, page_handle.slots, BITMAP_WIDTH, rid_.slot_no);
        if (nb == BITMAP_WIDTH) {
            if (file_handle_->file_hdr_.num_pages == rid_.page_no) {
                rid_.page_no = rid_.slot_no = -1;
                return;
            }
            page_handle = file_handle_->fetch_page_handle(++rid_.page_no);
        } else {
            rid_.slot_no = nb;
            return;
        }
    } while (1);
}

/**
 * @brief ​ 判断是否到达文件末尾
 */
bool RmScan::is_end() const {
    // Todo: 修改返回值
    RmPageHandle page_handle = file_handle_->fetch_page_handle(rid_.page_no);
    if (page_handle.file_hdr->num_pages == rid_.page_no && rid_.slot_no == page_handle.file_hdr->num_records_per_page - 1) {
        return true;
    }
    return false;
}

/**
 * @brief RmScan 内部存放的 rid
 */
Rid RmScan::rid() const {
    // Todo: 修改返回值
    return rid_;
}