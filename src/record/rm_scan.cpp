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
    rid_.page_no = RM_FIRST_RECORD_PAGE;
    rid_.slot_no = -1;
    next();
}

/**
 * @brief 找到文件中下一个存放了记录的位置
 */
void RmScan::next() {
    // Todo:
    // 找到文件中下一个存放了记录的非空闲位置，用 rid_来指向这个位置
    if (rid_.page_no == file_handle_->file_hdr_.num_pages) {
        rid_.page_no = RM_NO_PAGE;
        return;
    }
    while (1) {
        RmPageHandle page_handle = file_handle_->fetch_page_handle(rid_.page_no);
        int nb = Bitmap::next_bit(true, page_handle.bitmap, file_handle_->file_hdr_.num_records_per_page, rid_.slot_no);
        if (nb >= file_handle_->file_hdr_.num_records_per_page) {
            rid_.page_no ++;
            rid_.slot_no = -1;
            if (file_handle_->file_hdr_.num_pages == rid_.page_no) {
                rid_.page_no = rid_.slot_no = RM_NO_PAGE;
                return;
            }
            page_handle = file_handle_->fetch_page_handle(rid_.page_no);
        } else {
            rid_.slot_no = nb;
            return;
        }
    } 
}

/**
 * @brief ​ 判断是否到达文件末尾
 */
bool RmScan::is_end() const {
    // Todo: 修改返回值
    return rid_.page_no == RM_NO_PAGE;
}

/**
 * @brief RmScan 内部存放的 rid
 */
Rid RmScan::rid() const {
    // Todo: 修改返回值
    return rid_;
}