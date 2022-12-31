#include "ix_index_handle.h"

#include "ix_scan.h"

IxIndexHandle::IxIndexHandle(DiskManager *disk_manager, BufferPoolManager *buffer_pool_manager, int fd)
    : disk_manager_(disk_manager), buffer_pool_manager_(buffer_pool_manager), fd_(fd) {
    // init file_hdr_
    disk_manager_->read_page(fd, IX_FILE_HDR_PAGE, (char *)&file_hdr_, sizeof(file_hdr_));
    // disk_manager 管理的 fd 对应的文件中，设置从原来编号 +1 开始分配 page_no
    disk_manager_->set_fd2pageno(fd, disk_manager_->get_fd2pageno(fd) + 1);
}

/**
 * @brief 用于查找指定键所在的叶子结点
 *
 * @param key 要查找的目标 key 值
 * @param operation 查找到目标键值对后要进行的操作类型
 * @param transaction 事务参数，如果不需要则默认传入 nullptr
 * @return 返回目标叶子结点
 * @note need to Unpin the leaf node outside!
 */
IxNodeHandle *IxIndexHandle::FindLeafPage(const char *key, Operation operation, Transaction *transaction) {
    // Todo:
    // 1. 获取根节点
    // 2. 从根节点开始不断向下查找目标 key
    // 3. 找到包含该 key 值的叶子结点停止查找，并返回叶子节点
    IxNodeHandle *node = FetchNode(file_hdr_.root_page);
    while (!node->IsLeafPage()) {
        int ret = node->InternalLookup(key);
        node = FetchNode(ret);
    } 
    return node;
}

/**
 * @brief 用于查找指定键在叶子结点中的对应的值 result
 *
 * @param key 查找的目标 key 值
 * @param result 用于存放结果的容器
 * @param transaction 事务指针
 * @return bool 返回目标键值对是否存在
 */
bool IxIndexHandle::GetValue(const char *key, std::vector<Rid> *result, Transaction *transaction) {
    // Todo:
    // 1. 获取目标 key 值所在的叶子结点
    // 2. 在叶子节点中查找目标 key 值的位置，并读取 key 对应的 rid
    // 3. 把 rid 存入 result 参数中
    // 提示：使用完 buffer_pool 提供的 page 之后，记得 unpin page；记得处理并发的上锁
    std::scoped_lock lock{root_latch_};
    IxNodeHandle *node = FindLeafPage(key, Operation::FIND, nullptr);
    Rid *ret;
    if (node->LeafLookup(key, &ret)) {
        result->push_back(*ret);
        buffer_pool_manager_->UnpinPage(node->GetPageId(), false);
        return true;
    }
    buffer_pool_manager_->UnpinPage(node->GetPageId(), false);
    return false;
}

/**
 * @brief 将指定键值对插入到 B+树中
 *
 * @param (key, value) 要插入的键值对
 * @param transaction 事务指针
 * @return 是否插入成功
 */
bool IxIndexHandle::insert_entry(const char *key, const Rid &value, Transaction *transaction) {
    // Todo:
    // 1. 查找 key 值应该插入到哪个叶子节点
    // 2. 在该叶子节点中插入键值对
    // 3. 如果结点已满，分裂结点，并把新结点的相关信息插入父节点
    // 提示：记得 unpin page；若当前叶子节点是最右叶子节点，则需要更新 file_hdr_.last_leaf；记得处理并发的上锁

    return false;
}

/**
 * @brief 将传入的一个 node 拆分 (Split) 成两个结点，在 node 的右边生成一个新结点 new node
 *
 * @param node 需要拆分的结点
 * @return 拆分得到的 new_node
 * @note 本函数执行完毕后，原 node 和 new node 都需要在函数外面进行 unpin
 */
IxNodeHandle *IxIndexHandle::Split(IxNodeHandle *node) {
    // Todo:
    // 1. 将原结点的键值对平均分配，右半部分分裂为新的右兄弟结点
    //    需要初始化新节点的 page_hdr 内容
    // 2. 如果新的右兄弟结点是叶子结点，更新新旧节点的 prev_leaf 和 next_leaf 指针
    //    为新节点分配键值对，更新旧节点的键值对数记录
    // 3. 如果新的右兄弟结点不是叶子结点，更新该结点的所有孩子结点的父节点信息 (使用 IxIndexHandle::maintain_child())

    return nullptr;
}

/**
 * @brief Insert key & value pair into internal page after split
 * 拆分 (Split) 后，向上找到 old_node 的父结点
 * 将 new_node 的第一个 key 插入到父结点，其位置在 父结点指向 old_node 的孩子指针 之后
 * 如果插入后>=maxsize，则必须继续拆分父结点，然后在其父结点的父结点再插入，即需要递归
 * 直到找到的 old_node 为根结点时，结束递归（此时将会新建一个根 R，关键字为 key，old_node 和 new_node 为其孩子）
 *
 * @param (old_node, new_node) 原结点为 old_node，old_node 被分裂之后产生了新的右兄弟结点 new_node
 * @param key 要插入 parent 的 key
 * @note 一个结点插入了键值对之后需要分裂，分裂后左半部分的键值对保留在原结点，在参数中称为 old_node，
 * 右半部分的键值对分裂为新的右兄弟节点，在参数中称为 new_node（参考 Split 函数来理解 old_node 和 new_node）
 * @note 本函数执行完毕后，new node 和 old node 都需要在函数外面进行 unpin
 */
void IxIndexHandle::InsertIntoParent(IxNodeHandle *old_node, const char *key, IxNodeHandle *new_node,
                                     Transaction *transaction) {
    // Todo:
    // 1. 分裂前的结点（原结点，old_node）是否为根结点，如果为根结点需要分配新的 root
    // 2. 获取原结点（old_node）的父亲结点
    // 3. 获取 key 对应的 rid，并将 (key, rid) 插入到父亲结点
    // 4. 如果父亲结点仍需要继续分裂，则进行递归插入
    // 提示：记得 unpin page
}

/**
 * @brief 用于删除 B+树中含有指定 key 的键值对
 *
 * @param key 要删除的 key 值
 * @param transaction 事务指针
 * @return 是否删除成功
 */
bool IxIndexHandle::delete_entry(const char *key, Transaction *transaction) {
    // Todo:
    // 1. 获取该键值对所在的叶子结点
    // 2. 在该叶子结点中删除键值对
    // 3. 如果删除成功需要调用 CoalesceOrRedistribute 来进行合并或重分配操作，并根据函数返回结果判断是否有结点需要删除
    // 4. 如果需要并发，并且需要删除叶子结点，则需要在事务的 delete_page_set 中添加删除结点的对应页面；记得处理并发的上锁

    return false;
}

/**
 * @brief 用于处理合并和重分配的逻辑，用于删除键值对后调用
 *
 * @param node 执行完删除操作的结点
 * @param transaction 事务指针
 * @param root_is_latched 传出参数：根节点是否上锁，用于并发操作
 * @return 是否需要删除结点
 * @note User needs to first find the sibling of input page.
 * If sibling's size + input page's size >= 2 * page's minsize, then redistribute.
 * Otherwise, merge(Coalesce).
 */
bool IxIndexHandle::CoalesceOrRedistribute(IxNodeHandle *node, Transaction *transaction) {
    // Todo:
    // 1. 判断 node 结点是否为根节点
    //    1.1 如果是根节点，需要调用 AdjustRoot() 函数来进行处理，返回根节点是否需要被删除
    //    1.2 如果不是根节点，并且不需要执行合并或重分配操作，则直接返回 false，否则执行 2
    // 2. 获取 node 结点的父亲结点
    // 3. 寻找 node 结点的兄弟结点（优先选取前驱结点）
    // 4. 如果 node 结点和兄弟结点的键值对数量之和，能够支撑两个 B+树结点（即 node.size+neighbor.size >=
    // NodeMinSize*2)，则只需要重新分配键值对（调用 Redistribute 函数）
    // 5. 如果不满足上述条件，则需要合并两个结点，将右边的结点合并到左边的结点（调用 Coalesce 函数）

    return false;
}

/**
 * @brief 用于当根结点被删除了一个键值对之后的处理
 *
 * @param old_root_node 原根节点
 * @return bool 根结点是否需要被删除
 * @note size of root page can be less than min size and this method is only called within coalesceOrRedistribute()
 */
bool IxIndexHandle::AdjustRoot(IxNodeHandle *old_root_node) {
    // Todo:
    // 1. 如果 old_root_node 是内部结点，并且大小为 1，则直接把它的孩子更新成新的根结点
    // 2. 如果 old_root_node 是叶结点，且大小为 0，则直接更新 root page
    // 3. 除了上述两种情况，不需要进行操作

    return false;
}

/**
 * @brief 重新分配 node 和兄弟结点 neighbor_node 的键值对
 * Redistribute key & value pairs from one page to its sibling page. If index == 0, move sibling page's first key
 * & value pair into end of input "node", otherwise move sibling page's last key & value pair into head of input "node".
 *
 * @param neighbor_node sibling page of input "node"
 * @param node input from method coalesceOrRedistribute()
 * @param parent the parent of "node" and "neighbor_node"
 * @param index node 在 parent 中的 rid_idx
 * @note node 是之前刚被删除过一个 key 的结点
 * index=0，则 neighbor 是 node 后继结点，表示：node(left)      neighbor(right)
 * index>0，则 neighbor 是 node 前驱结点，表示：neighbor(left)  node(right)
 * 注意更新 parent 结点的相关 kv 对
 */
void IxIndexHandle::Redistribute(IxNodeHandle *neighbor_node, IxNodeHandle *node, IxNodeHandle *parent, int index) {
    // Todo:
    // 1. 通过 index 判断 neighbor_node 是否为 node 的前驱结点
    // 2. 从 neighbor_node 中移动一个键值对到 node 结点中
    // 3. 更新父节点中的相关信息，并且修改移动键值对对应孩字结点的父结点信息（maintain_child 函数）
    // 注意：neighbor_node 的位置不同，需要移动的键值对不同，需要分类讨论

}

/**
 * @brief 合并 (Coalesce) 函数是将 node 和其直接前驱进行合并，也就是和它左边的 neighbor_node 进行合并；
 * 假设 node 一定在右边。如果上层传入的 index=0，说明 node 在左边，那么交换 node 和 neighbor_node，保证 node 在右边；合并到左结点，实际上就是删除了右结点；
 * Move all the key & value pairs from one page to its sibling page, and notify buffer pool manager to delete this page.
 * Parent page must be adjusted to take info of deletion into account. Remember to deal with coalesce or redistribute
 * recursively if necessary.
 *
 * @param neighbor_node sibling page of input "node" (neighbor_node 是 node 的前结点)
 * @param node input from method coalesceOrRedistribute() (node 结点是需要被删除的)
 * @param parent parent page of input "node"
 * @param index node 在 parent 中的 rid_idx
 * @return true means parent node should be deleted, false means no deletion happend
 * @note Assume that *neighbor_node is the left sibling of *node (neighbor -> node)
 */
bool IxIndexHandle::Coalesce(IxNodeHandle **neighbor_node, IxNodeHandle **node, IxNodeHandle **parent, int index,
                             Transaction *transaction) {
    // Todo:
    // 1. 用 index 判断 neighbor_node 是否为 node 的前驱结点，若不是则交换两个结点，让 neighbor_node 作为左结点，node 作为右结点
    // 2. 把 node 结点的键值对移动到 neighbor_node 中，并更新 node 结点孩子结点的父节点信息（调用 maintain_child 函数）
    // 3. 释放和删除 node 结点，并删除 parent 中 node 结点的信息，返回 parent 是否需要被删除
    // 提示：如果是叶子结点且为最右叶子结点，需要更新 file_hdr_.last_leaf

    return false;
}

/** -- 以下为辅助函数 -- */
/**
 * @brief 获取一个指定结点
 *
 * @param page_no
 * @return IxNodeHandle*
 * @note pin the page, remember to unpin it outside!
 */
IxNodeHandle *IxIndexHandle::FetchNode(int page_no) const {
    // assert(page_no < file_hdr_.num_pages); // 不再生效，由于删除操作，page_no 可以大于个数
    Page *page = buffer_pool_manager_->FetchPage(PageId{fd_, page_no});
    IxNodeHandle *node = new IxNodeHandle(&file_hdr_, page);
    return node;
}

/**
 * @brief 创建一个新结点
 *
 * @return IxNodeHandle*
 * @note pin the page, remember to unpin it outside!
 * 注意：对于 Index 的处理是，删除某个页面后，认为该被删除的页面是 free_page
 * 而 first_free_page 实际上就是最新被删除的页面，初始为 IX_NO_PAGE
 * 在最开始插入时，一直是 create node，那么 first_page_no 一直没变，一直是 IX_NO_PAGE
 * 与 Record 的处理不同，Record 将未插入满的记录页认为是 free_page
 */
IxNodeHandle *IxIndexHandle::CreateNode() {
    file_hdr_.num_pages++;
    PageId new_page_id = {.fd = fd_, .page_no = INVALID_PAGE_ID};
    // 从 3 开始分配 page_no，第一次分配之后，new_page_id.page_no=3，file_hdr_.num_pages=4
    Page *page = buffer_pool_manager_->NewPage(&new_page_id);
    // 注意，和 Record 的 free_page 定义不同，此处【不能】加上：file_hdr_.first_free_page_no = page->GetPageId().page_no
    IxNodeHandle *node = new IxNodeHandle(&file_hdr_, page);
    return node;
}

/**
 * @brief 从 node 开始更新其父节点的第一个 key，一直向上更新直到根节点
 *
 * @param node
 */
void IxIndexHandle::maintain_parent(IxNodeHandle *node) {
    IxNodeHandle *curr = node;
    while (curr->GetParentPageNo() != IX_NO_PAGE) {
        // Load its parent
        IxNodeHandle *parent = FetchNode(curr->GetParentPageNo());
        int rank = parent->find_child(curr);
        char *parent_key = parent->get_key(rank);
        // char *child_max_key = curr.get_key(curr.page_hdr->num_key - 1);
        char *child_first_key = curr->get_key(0);
        if (memcmp(parent_key, child_first_key, file_hdr_.col_len) == 0) {
            assert(buffer_pool_manager_->UnpinPage(parent->GetPageId(), true));
            break;
        }
        memcpy(parent_key, child_first_key, file_hdr_.col_len);  // 修改了 parent node
        curr = parent;

        assert(buffer_pool_manager_->UnpinPage(parent->GetPageId(), true));
    }
}

/**
 * @brief 要删除 leaf 之前调用此函数，更新 leaf 前驱结点的 next 指针和后继结点的 prev 指针
 *
 * @param leaf 要删除的 leaf
 */
void IxIndexHandle::erase_leaf(IxNodeHandle *leaf) {
    assert(leaf->IsLeafPage());

    IxNodeHandle *prev = FetchNode(leaf->GetPrevLeaf());
    prev->SetNextLeaf(leaf->GetNextLeaf());
    buffer_pool_manager_->UnpinPage(prev->GetPageId(), true);

    IxNodeHandle *next = FetchNode(leaf->GetNextLeaf());
    next->SetPrevLeaf(leaf->GetPrevLeaf());  // 注意此处是 SetPrevLeaf()
    buffer_pool_manager_->UnpinPage(next->GetPageId(), true);
}

/**
 * @brief 删除 node 时，更新 file_hdr_.num_pages
 *
 * @param node
 */
void IxIndexHandle::release_node_handle(IxNodeHandle &node) { file_hdr_.num_pages--; }

/**
 * @brief 将 node 的第 child_idx 个孩子结点的父节点置为 node
 */
void IxIndexHandle::maintain_child(IxNodeHandle *node, int child_idx) {
    if (!node->IsLeafPage()) {
        //  Current node is inner node, load its child and set its parent to current node
        int child_page_no = node->ValueAt(child_idx);
        IxNodeHandle *child = FetchNode(child_page_no);
        child->SetParentPageNo(node->GetPageNo());
        buffer_pool_manager_->UnpinPage(child->GetPageId(), true);
    }
}

/**
 * @brief 这里把 iid 转换成了 rid，即 iid 的 slot_no 作为 node 的 rid_idx(key_idx)
 * node 其实就是把 slot_no 作为键值对数组的下标
 * 换而言之，每个 iid 对应的索引槽存了一对 (key,rid)，指向了 (要建立索引的属性首地址，插入/删除记录的位置)
 *
 * @param iid
 * @return Rid
 * @note iid 和 rid 存的不是一个东西，rid 是上层传过来的记录位置，iid 是索引内部生成的索引槽位置
 */
Rid IxIndexHandle::get_rid(const Iid &iid) const {
    IxNodeHandle *node = FetchNode(iid.page_no);
    if (iid.slot_no >= node->GetSize()) {
        throw IndexEntryNotFoundError();
    }
    buffer_pool_manager_->UnpinPage(node->GetPageId(), false);  // unpin it!
    return *node->get_rid(iid.slot_no);
}

/** --以下函数将用于 lab3 执行层-- */
/**
 * @brief FindLeafPage + lower_bound
 *
 * @param key
 * @return Iid
 * @note 上层传入的 key 本来是 int 类型，通过 (const char *)&key 进行了转换
 * 可用*(int *)key 转换回去
 */
Iid IxIndexHandle::lower_bound(const char *key) {
    // int int_key = *(int *)key;
    // printf("my_lower_bound key=%d\n", int_key);

    IxNodeHandle *node = FindLeafPage(key, Operation::FIND, nullptr);
    int key_idx = node->lower_bound(key);

    Iid iid = {.page_no = node->GetPageNo(), .slot_no = key_idx};

    // unpin leaf node
    buffer_pool_manager_->UnpinPage(node->GetPageId(), false);
    return iid;
}

/**
 * @brief FindLeafPage + upper_bound
 *
 * @param key
 * @return Iid
 */
Iid IxIndexHandle::upper_bound(const char *key) {
    // int int_key = *(int *)key;
    // printf("my_upper_bound key=%d\n", int_key);

    IxNodeHandle *node = FindLeafPage(key, Operation::FIND, nullptr);
    int key_idx = node->upper_bound(key);

    Iid iid;
    if (key_idx == node->GetSize()) {
        // 这种情况无法根据 iid 找到 rid，即后续无法调用 ih->get_rid(iid)
        iid = leaf_end();
    } else {
        iid = {.page_no = node->GetPageNo(), .slot_no = key_idx};
    }

    // unpin leaf node
    buffer_pool_manager_->UnpinPage(node->GetPageId(), false);
    return iid;
}

/**
 * @brief 指向第一个叶子的第一个结点
 * 用处在于可以作为 IxScan 的第一个
 *
 * @return Iid
 */
Iid IxIndexHandle::leaf_begin() const {
    Iid iid = {.page_no = file_hdr_.first_leaf, .slot_no = 0};
    return iid;
}

/**
 * @brief 指向最后一个叶子的最后一个结点的后一个
 * 用处在于可以作为 IxScan 的最后一个
 *
 * @return Iid
 */
Iid IxIndexHandle::leaf_end() const {
    IxNodeHandle *node = FetchNode(file_hdr_.last_leaf);
    Iid iid = {.page_no = file_hdr_.last_leaf, .slot_no = node->GetSize()};
    buffer_pool_manager_->UnpinPage(node->GetPageId(), false);  // unpin it!
    return iid;
}
