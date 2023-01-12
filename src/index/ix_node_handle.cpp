#include "ix_node_handle.h"

/**
 * @brief 在当前 node 中查找第一个>=target 的 key_idx
 *
 * @return key_idx，范围为 [0,num_key)，如果返回的 key_idx=num_key，则表示 target 大于最后一个 key
 * @note 返回 key index（同时也是 rid index），作为 slot no
 */
int IxNodeHandle::lower_bound(const char *target) const {
    // Todo:
    // 查找当前节点中第一个大于等于 target 的 key，并返回 key 的位置给上层
    // 提示：可以采用多种查找方式，如顺序遍历、二分查找等；使用 ix_compare() 函数进行比较
    int l = 0, r = page_hdr->num_key;
    while (l < r) {
        int mid = l + r >> 1;
        if (ix_compare(target, get_key(mid), file_hdr->col_type, file_hdr->col_len) > 0) {
            l = mid + 1;
        } else {
            r = mid;
        }
    }
    return l;
}

/**
 * @brief 在当前 node 中查找第一个>target 的 key_idx
 *
 * @return key_idx，范围为 [1,num_key)，如果返回的 key_idx=num_key，则表示 target 大于等于最后一个 key
 * @note 注意此处的范围从 1 开始
 */
int IxNodeHandle::upper_bound(const char *target) const {
    // Todo:
    // 查找当前节点中第一个大于 target 的 key，并返回 key 的位置给上层
    // 提示：可以采用多种查找方式：顺序遍历、二分查找等；使用 ix_compare() 函数进行比较
    int l = 1, r = page_hdr->num_key;
    while (l < r) {
        int mid = l + r >> 1;
        if (ix_compare(target, get_key(mid),  file_hdr->col_type, file_hdr->col_len) >= 0) {
            l = mid + 1;
        } else {
            r = mid;
        }
    }
    return l;
}

/**
 * @brief 用于叶子结点根据 key 来查找该结点中的键值对
 * 值 value 作为传出参数，函数返回是否查找成功
 *
 * @param key 目标 key
 * @param[out] value 传出参数，目标 key 对应的 Rid
 * @return 目标 key 是否存在
 */
bool IxNodeHandle::LeafLookup(const char *key, Rid **value) {
    // Todo:
    // 1. 在叶子节点中获取目标 key 所在位置
    // 2. 判断目标 key 是否存在
    // 3. 如果存在，获取 key 对应的 Rid，并赋值给传出参数 value
    // 提示：可以调用 lower_bound() 和 get_rid() 函数。
    int slot_no = IxNodeHandle::lower_bound(key);
    if (slot_no != page_hdr->num_key && ix_compare(get_key(slot_no), key, file_hdr->col_type, file_hdr->col_len) == 0) {
        Rid *rid = get_rid(slot_no);
        *value = rid;   //修改指针的值
        return true;
    }
    return false;
}

/**
 * 用于内部结点（非叶子节点）查找目标 key 所在的孩子结点（子树）
 * @param key 目标 key
 * @return page_id_t 目标 key 所在的孩子节点（子树）的存储页面编号
 */
page_id_t IxNodeHandle::InternalLookup(const char *key) {
    // Todo:
    // 1. 查找当前非叶子节点中目标 key 所在孩子节点（子树）的位置
    // 2. 获取该孩子节点（子树）所在页面的编号
    // 3. 返回页面编号
    int slot_no = upper_bound(key);
    slot_no--;
    return get_rid(slot_no)->page_no;
}

/**
 * @brief 在指定位置插入 n 个连续的键值对
 * 将 key 的前 n 位插入到原来 keys 中的 pos 位置；将 rid 的前 n 位插入到原来 rids 中的 pos 位置
 *
 * @param pos 要插入键值对的位置
 * @param (key, rid) 连续键值对的起始地址，也就是第一个键值对，可以通过 (key, rid) 来获取 n 个键值对
 * @param n 键值对数量
 * @note [0,pos)           [pos,num_key)
 *                            key_slot
 *                            /      \
 *                           /        \
 *       [0,pos)     [pos,pos+n)   [pos+n,num_key+n)
 *                      key           key_slot
 */
void IxNodeHandle::insert_pairs(int pos, const char *key, const Rid *rid, int n) {
    // Todo:
    // 1. 判断 pos 的合法性
    // 2. 通过 key 获取 n 个连续键值对的 key 值，并把 n 个 key 值插入到 pos 位置
    // 3. 通过 rid 获取 n 个连续键值对的 rid 值，并把 n 个 rid 值插入到 pos 位置
    // 4. 更新当前节点的键数量
    assert (pos >= 0 && pos <= page_hdr->num_key);
    int new_num_key = page_hdr->num_key + n;
    int copy_num = page_hdr->num_key - pos;
    int new_pos = pos + n;
    char *new_key = get_key(new_pos), *old_key = get_key(pos);
    Rid *new_rid = get_rid(new_pos), *old_rid = get_rid(pos);
    memmove(new_key, old_key, copy_num * file_hdr->col_len);
    memmove(new_rid, old_rid, copy_num * sizeof(Rid));
    memcpy(old_key, key, n * file_hdr->col_len);
    memcpy(old_rid, rid, n * sizeof(Rid)); 
    page_hdr->num_key = new_num_key;
}

/**
 * @brief 用于在结点中的指定位置插入单个键值对
 */
void IxNodeHandle::insert_pair(int pos, const char *key, const Rid &rid) { insert_pairs(pos, key, &rid, 1); };

/**
 * @brief 用于在结点中插入单个键值对。
 * 函数返回插入后的键值对数量
 *
 * @param (key, value) 要插入的键值对
 * @return int 键值对数量
 */
int IxNodeHandle::Insert(const char *key, const Rid &value) {
    // Todo:
    // 1. 查找要插入的键值对应该插入到当前节点的哪个位置
    // 2. 如果 key 重复则不插入
    // 3. 如果 key 不重复则插入键值对
    // 4. 返回完成插入操作之后的键值对数量
    int idx = lower_bound(key);
    if (idx != page_hdr->num_key && ix_compare(get_key(idx), key, file_hdr->col_type, file_hdr->col_len) == 0) {
        return page_hdr->num_key;
    }
    insert_pair(idx, key, value);
    return page_hdr->num_key;
}

/**
 * @brief 用于在结点中的指定位置删除单个键值对
 *
 * @param pos 要删除键值对的位置
 */
void IxNodeHandle::erase_pair(int pos) {
    // Todo:
    // 1. 删除该位置的 key
    // 2. 删除该位置的 rid
    // 3. 更新结点的键值对数量
    assert (pos >= 0 && pos < page_hdr->num_key);
    char *old_key = get_key(pos), *new_key = get_key(pos+1);
    Rid *old_rid = get_rid(pos), *new_rid = get_rid(pos+1);
    memmove(old_key, new_key, (page_hdr->num_key - 1 - pos) * file_hdr->col_len);
    memmove(old_rid, new_rid, (page_hdr->num_key - 1 - pos) * sizeof(Rid));
    page_hdr->num_key--;
}

/**
 * @brief 用于在结点中删除指定 key 的键值对。函数返回删除后的键值对数量
 *
 * @param key 要删除的键值对 key 值
 * @return 完成删除操作后的键值对数量
 */
int IxNodeHandle::Remove(const char *key) {
    // Todo:
    // 1. 查找要删除键值对的位置
    // 2. 如果要删除的键值对存在，删除键值对
    // 3. 返回完成删除操作后的键值对数量
    int pos = lower_bound(key);
    if ((pos != page_hdr->num_key) && (ix_compare(key, get_key(pos), file_hdr->col_type, file_hdr->col_len) == 0)) {
        erase_pair(pos);
    }
    return page_hdr->num_key;
}

/**
 * @brief 由 parent 调用，寻找 child，返回 child 在 parent 中的 rid_idx∈[0,page_hdr->num_key)
 *
 * @param child
 * @return int
 */
int IxNodeHandle::find_child(IxNodeHandle *child) {
    int rid_idx;
    for (rid_idx = 0; rid_idx < page_hdr->num_key; rid_idx++) {
        if (get_rid(rid_idx)->page_no == child->GetPageNo()) {
            break;
        }
    }
    assert(rid_idx < page_hdr->num_key);
    return rid_idx;
}

/**
 * @brief used in internal node to remove the last key in root node, and return the last child
 *
 * @return the last child
 */
page_id_t IxNodeHandle::RemoveAndReturnOnlyChild() {
    assert(GetSize() == 1);
    page_id_t child_page_no = ValueAt(0);
    erase_pair(0);
    assert(GetSize() == 0);
    return child_page_no;
}