#pragma once

#include <assert.h>

#include <memory>

#include "bitmap.h"
#include "common/context.h"
#include "rm_defs.h"

class RmManager;

// 对单个 page 进行封装，用 page 中的 data 存 RmPageHdr, bitmap, slots 的数据
struct RmPageHandle {
    const RmFileHdr *file_hdr;  // 用到了 file_hdr 的 bitmap_size, record_size
    Page *page;                 // 指向单个 page
    RmPageHdr *page_hdr;        // page->data 的第一部分，指针指向首地址，长度为 sizeof(RmPageHdr)
    char *bitmap;               // page->data 的第二部分，指针指向首地址，长度为 file_hdr->bitmap_size
    char *slots;  // page->data 的第三部分，指针指向首地址，每个 slot 的长度为 file_hdr->record_size

    RmPageHandle(const RmFileHdr *fhdr_, Page *page_) : file_hdr(fhdr_), page(page_) {
        page_hdr = reinterpret_cast<RmPageHdr *>(page->GetData() + page->OFFSET_PAGE_HDR);
        bitmap = page->GetData() + sizeof(RmPageHdr) + page->OFFSET_PAGE_HDR;
        slots = bitmap + file_hdr->bitmap_size;
    }

    // 返回位于 slot_no 的 record 的地址
    char *get_slot(int slot_no) const {
        return slots + slot_no * file_hdr->record_size;  // slots 的首地址 + slot 个数 * 每个 slot 的大小 (每个 record 的大小)
    }
};

// 每个 RmFileHandle 对应一个文件，里面有多个 page，每个 page 的数据封装在 RmPageHandle
class RmFileHandle {      // TableHeap
    friend class RmScan;  // TableIterator
    friend class RmManager;

   private:
    DiskManager *disk_manager_;
    BufferPoolManager *buffer_pool_manager_;
    int fd_;
    /** @brief file_hdr中的num_pages记录此文件分配的page个数
     * page_no范围为[0,file_hdr.num_pages)，page_no从0开始增加，其中第0页存file_hdr，从第1页开始存page_handle
     * 在page_handle中有page_hdr.free_page_no存第一个可用(未满)的page_no
     * */
    RmFileHdr file_hdr_;

   public:
    RmFileHandle(DiskManager *disk_manager, BufferPoolManager *buffer_pool_manager, int fd)
        : disk_manager_(disk_manager), buffer_pool_manager_(buffer_pool_manager), fd_(fd) {
        // 注意：这里从磁盘中读出文件描述符为 fd 的文件的 file_hdr，读到内存中
        // 这里实际就是初始化 file_hdr，只不过是从磁盘中读出进行初始化
        // init file_hdr_
        disk_manager_->read_page(fd, RM_FILE_HDR_PAGE, (char *)&file_hdr_, sizeof(file_hdr_));
        // disk_manager 管理的 fd 对应的文件中，设置从 file_hdr_.num_pages 开始分配 page_no
        disk_manager_->set_fd2pageno(fd, file_hdr_.num_pages);
    }

    DISALLOW_COPY(RmFileHandle);
    // RmFileHandle(const RmFileHandle &other) = delete;
    // RmFileHandle &operator=(const RmFileHandle &other) = delete;

    RmFileHdr get_file_hdr() { return file_hdr_; }
    int GetFd() { return fd_; }

    bool is_record(const Rid &rid) const {
        RmPageHandle page_handle = fetch_page_handle(rid.page_no);
        return Bitmap::is_set(page_handle.bitmap, rid.slot_no);  // page 的 slot_no 位置上是否有 record
    }

    std::unique_ptr<RmRecord> get_record(const Rid &rid, Context *context) const;

    Rid insert_record(char *buf, Context *context);

    void insert_record(const Rid &rid, char *buf);

    void delete_record(const Rid &rid, Context *context);

    void update_record(const Rid &rid, char *buf, Context *context);

    RmPageHandle create_new_page_handle();

    RmPageHandle fetch_page_handle(int page_no) const;

   private:
    RmPageHandle create_page_handle();

    void release_page_handle(RmPageHandle &page_handle);
};
