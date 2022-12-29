#include "rm_file_handle.h"

/**
 * @brief 由 Rid 得到指向 RmRecord 的指针
 *
 * @param rid 指定记录所在的位置
 * @return std::unique_ptr<RmRecord>
 */
std::unique_ptr<RmRecord> RmFileHandle::get_record(const Rid &rid, Context *context) const {
    // Todo:
    // 1. 获取指定记录所在的 page handle
    // 2. 初始化一个指向 RmRecord 的指针（赋值其内部的 data 和 size）
    //context 怎么用？
    std::unique_ptr<RmRecord> p = std::make_unique<RmRecord>(file_hdr_.record_size);
    RmPageHandle page_handle = fetch_page_handle(rid.page_no);
    if (!Bitmap::is_set(page_handle.bitmap, rid.slot_no)) {
        throw RecordNotFoundError(rid.page_no, rid.slot_no);
    }
    char *slot = page_handle.get_slot(rid.slot_no);
    p->size = file_hdr_.record_size;
    memcpy(p->data, slot, file_hdr_.record_size);
    return p;
}

/**
 * @brief 在该记录文件（RmFileHandle）中插入一条记录
 *
 * @param buf 要插入的数据的地址
 * @return Rid 插入记录的位置
 */
Rid RmFileHandle::insert_record(char *buf, Context *context) {
    // Todo:
    // 1. 获取当前未满的 page handle
    // 2. 在 page handle 中找到空闲 slot 位置
    // 3. 将 buf 复制到空闲 slot 位置
    // 4. 更新 page_handle.page_hdr 中的数据结构
    // 注意考虑插入一条记录后页面已满的情况，需要更新 file_hdr_.first_free_page_no
    RmPageHandle page_handle = create_page_handle();
    Rid ret;
    ret.slot_no = Bitmap::next_bit(false, page_handle.bitmap, file_hdr_.num_records_per_page, -1);
    Bitmap::set(page_handle.bitmap, ret.slot_no);
    char *slot = page_handle.get_slot(ret.slot_no);
    memcpy(slot, buf, file_hdr_.record_size);
    page_handle.page_hdr->num_records++;
    if (page_handle.page_hdr->num_records == file_hdr_.num_records_per_page ) {
        //if (file_hdr_.first_free_page_no == page_handle.page->GetPageId().page_no)
        //因为插入这一条之后当前页满了，所以 first_free_page_no 原先一定是当前页，更新其为下一页
            file_hdr_.first_free_page_no = page_handle.page_hdr->next_free_page_no; 
    }
    ret.page_no = page_handle.page->GetPageId().page_no;
    return ret;
}

/**
 * @brief 在该记录文件（RmFileHandle）中删除一条指定位置的记录
 *
 * @param rid 要删除的记录所在的指定位置
 */
void RmFileHandle::delete_record(const Rid &rid, Context *context) {
    // Todo:
    // 1. 获取指定记录所在的 page handle
    // 2. 更新 page_handle.page_hdr 中的数据结构
    // 注意考虑删除一条记录后页面未满的情况，需要调用 release_page_handle()
    RmPageHandle page_handle = fetch_page_handle(rid.page_no);
    if (!Bitmap::is_set(page_handle.bitmap, rid.slot_no)) {
        throw RecordNotFoundError(rid.page_no, rid.slot_no);
    }
    //memset(page_handle.get_slot(rid.slot_no), 0, file_hdr_.record_size);
    Bitmap::reset(page_handle.bitmap, rid.slot_no);
    page_handle.page_hdr->num_records--;
    if (page_handle.page_hdr->num_records == file_hdr_.num_records_per_page - 1) {
        release_page_handle(page_handle);
    }
}

/**
 * @brief 更新指定位置的记录
 *
 * @param rid 指定位置的记录
 * @param buf 新记录的数据的地址
 */
void RmFileHandle::update_record(const Rid &rid, char *buf, Context *context) {
    // Todo:
    // 1. 获取指定记录所在的 page handle
    // 2. 更新记录
    RmPageHandle page_handle = fetch_page_handle(rid.page_no);
    if (!Bitmap::is_set(page_handle.bitmap, rid.slot_no)) {
        throw RecordNotFoundError(rid.page_no, rid.slot_no);
    }
    char *slot = page_handle.get_slot(rid.slot_no);
    memcpy(slot, buf, file_hdr_.record_size);
}

/** -- 以下为辅助函数 -- */
/**
 * @brief 获取指定页面编号的 page handle
 *
 * @param page_no 要获取的页面编号
 * @return RmPageHandle 返回给上层的 page_handle
 * @note pin the page, remember to unpin it outside!
 */
RmPageHandle RmFileHandle::fetch_page_handle(int page_no) const {
    // Todo:
    // 使用缓冲池获取指定页面，并生成 page_handle 返回给上层
    // if page_no is invalid, throw PageNotExistError exception
    if (page_no == INVALID_PAGE_ID || page_no >= file_hdr_.num_pages) 
        throw PageNotExistError("不知道表名是什么", page_no);
    PageId page_id = PageId();
    page_id.fd = fd_;
    page_id.page_no = page_no;
    Page *page = buffer_pool_manager_->FetchPage(page_id);
    return RmPageHandle(&file_hdr_, page);
}

/**
 * @brief 创建一个新的 page handle
 *
 * @return RmPageHandle
 */
RmPageHandle RmFileHandle::create_new_page_handle() {
    // Todo:
    // 1.使用缓冲池来创建一个新 page
    // 2.更新 page handle 中的相关信息
    // 3.更新 file_hdr
    PageId *page_id = new PageId;
    page_id->fd = fd_;
    Page *page = buffer_pool_manager_->NewPage(page_id);
    RmPageHandle page_handle = RmPageHandle(&file_hdr_, page);
    page_handle.page_hdr->next_free_page_no = RM_NO_PAGE;
    page_handle.page_hdr->num_records = 0;
    Bitmap::init(page_handle.bitmap, file_hdr_.bitmap_size);
    file_hdr_.first_free_page_no = page->GetPageId().page_no;
    file_hdr_.num_pages++;
    return page_handle;
}

/**
 * @brief 创建或获取一个空闲的 page handle
 *
 * @return RmPageHandle 返回生成的空闲 page handle
 * @note pin the page, remember to unpin it outside!
 */
RmPageHandle RmFileHandle::create_page_handle() {
    // Todo:
    // 1. 判断 file_hdr_中是否还有空闲页
    //     1.1 没有空闲页：使用缓冲池来创建一个新 page；可直接调用 create_new_page_handle()
    //     1.2 有空闲页：直接获取第一个空闲页
    // 2. 生成 page handle 并返回给上层
    if (file_hdr_.first_free_page_no != RM_NO_PAGE) {
        return fetch_page_handle(file_hdr_.first_free_page_no);
    } else {
        return create_new_page_handle();
    }
}

/**
 * @brief 当 page handle 中的 page 从已满变成未满的时候调用
 *
 * @param page_handle
 * @note only used in delete_record()
 */
void RmFileHandle::release_page_handle(RmPageHandle &page_handle) {
    // Todo:
    // 当 page 从已满变成未满，考虑如何更新：
    // 1. page_handle.page_hdr->next_free_page_no
    // 2. file_hdr_.first_free_page_no
    page_handle.page_hdr->next_free_page_no = file_hdr_.first_free_page_no;
    file_hdr_.first_free_page_no = page_handle.page->GetPageId().page_no;
    //}
}

// used for recovery (lab4)
void RmFileHandle::insert_record(const Rid &rid, char *buf) {
    if (rid.page_no < file_hdr_.num_pages) {
        create_new_page_handle();
    }
    RmPageHandle pageHandle = fetch_page_handle(rid.page_no);
    Bitmap::set(pageHandle.bitmap, rid.slot_no);
    pageHandle.page_hdr->num_records++;
    if (pageHandle.page_hdr->num_records == file_hdr_.num_records_per_page) {
        file_hdr_.first_free_page_no = pageHandle.page_hdr->next_free_page_no;
    }

    char *slot = pageHandle.get_slot(rid.slot_no);
    memcpy(slot, buf, file_hdr_.record_size);

    buffer_pool_manager_->UnpinPage(pageHandle.page->GetPageId(), true);
}