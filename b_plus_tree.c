#include <stdio.h>
#include <memory.h>
#include "b_plus_tree.h"
extern struct b_plus_tree b_plus_tree;
extern Node *first_leaf_node;

int Delete(index_t key)
{
    Node *work_node = b_plus_tree.root_node;
    int delete_pos;
    while (!work_node->leaf)
    {
        delete_pos = SearchPos(work_node, key, true);
        if (delete_pos == LESS_THAN_MIN)
        {
            return -1;
        }
        work_node = work_node->child_node_ptr[delete_pos];
    }

    if (DeleteLeafNodeOneItem(work_node, key) == -1)
    {
        return -1;
    }
    return 0;
}
int DeleteLeafNodeOneItem(Node *work_node, index_t key)
{
    int delete_pos = SearchPos(work_node, key, true);
    if (delete_pos >= 0)
    {
        return -1;
    }
    else
    {
        delete_pos = -1 * (delete_pos + 1);
        index_t old_key = work_node->keys[0];
        for (int i = delete_pos; i < work_node->child_num; i++)
        {
            work_node->keys[i] = work_node->keys[i + 1];
            work_node->values[i] = work_node->values[i + 1];
        }
        work_node->child_num--;
        b_plus_tree.leaf_nums--;
        if (delete_pos == 0)
        {
            UpdateKey(work_node, old_key);
        }
        if (work_node->child_num < MIN_CACHE_NUM)
        {
            if (LendLeafNode(work_node) == 0)
            {
                return 0;
            }
            else
            {
                MergeLeafNode(work_node);
            }
        }
    }
    return 0;
}
int DeleteInternalNodeOneItem(Node *work_node, index_t key)
{
    int delete_pos = SearchPos(work_node, key, true);
    // if (delete_pos >= 0)
    // {
    //     return -1;
    // }
    // else
    // {
    // delete_pos = -1 * (delete_pos + 1);
    index_t old_key = work_node->keys[0];
    for (int i = delete_pos; i < work_node->child_num - 1; i++)
    {
        work_node->keys[i] = work_node->keys[i + 1];
        work_node->child_node_ptr[i] = work_node->child_node_ptr[i + 1];
    }
    work_node->child_num--;
    if (delete_pos == 0)
    {
        UpdateKey(work_node, old_key);
    }

    if (work_node == b_plus_tree.root_node)
    {
        return 0;
    }
    if (work_node->child_num < MIN_CACHE_NUM)
    {
        if (LendInternalNode(work_node) == 0)
        {
            return 0;
        }
        else
        {
            MergeInternalNode(work_node);
        }
    }

    // }
    return 0;
}
int LendInternalNode(Node *work_node)
{
    Node *lend_from;
    if (work_node->next_node && work_node->next_node->child_num > MIN_CACHE_NUM)
    {
        lend_from = work_node->next_node;
        work_node->keys[work_node->child_num] = lend_from->keys[0];
        work_node->child_node_ptr[work_node->child_num] = lend_from->child_node_ptr[0];
        work_node->child_node_ptr[work_node->child_num]->parent_ptr=work_node;
        for (int i = 0; i < lend_from->child_num - 1; i++)
        {
            lend_from->keys[i] = lend_from->keys[i + 1];
            lend_from->child_node_ptr[i] = lend_from->child_node_ptr[i + 1];
        }
        work_node->child_num++;
        lend_from->child_num--;
        UpdateKey(lend_from, work_node->keys[work_node->child_num - 1]);
    }
    else if (work_node->pre_node && work_node->pre_node->child_num > MIN_CACHE_NUM)
    {
        lend_from = work_node->pre_node;
        for (int i = work_node->child_num - 1; i >= 0; i--)
        {
            work_node->keys[i + 1] = work_node->keys[i];
            work_node->child_node_ptr[i + 1] = work_node->child_node_ptr[i];
        }
        work_node->keys[0] = lend_from->keys[lend_from->child_num - 1];
        work_node->child_node_ptr[0] = lend_from->child_node_ptr[lend_from->child_num - 1];
        work_node->child_node_ptr[0]->parent_ptr=work_node;

        lend_from->child_num--;
        work_node->child_num++;
        UpdateKey(work_node, work_node->keys[1]);
    }
    else
    {
        return -1;
    }
    return 0;
}
int LendLeafNode(Node *work_node)
{
    Node *lend_from;
    if (work_node->next_node && work_node->next_node->child_num > MIN_CACHE_NUM)
    {
        lend_from = work_node->next_node;
        work_node->keys[work_node->child_num] = lend_from->keys[0];
        work_node->values[work_node->child_num] = lend_from->values[0];
        for (int i = 0; i < lend_from->child_num - 1; i++)
        {
            lend_from->keys[i] = lend_from->keys[i + 1];
            lend_from->values[i] = lend_from->values[i + 1];
        }
        work_node->child_num++;
        lend_from->child_num--;
        UpdateKey(lend_from, work_node->keys[work_node->child_num]);
    }
    else if (work_node->pre_node && work_node->pre_node->child_num > MIN_CACHE_NUM)
    {
        lend_from = work_node->pre_node;
        for (int i = work_node->child_num - 1; i >= 0; i--)
        {
            work_node->keys[i + 1] = work_node->keys[i];
            work_node->values[i + 1] = work_node->values[i];
        }
        work_node->keys[0] = lend_from->keys[lend_from->child_num - 1];
        work_node->values[0] = lend_from->values[lend_from->child_num - 1];
        lend_from->child_num--;
        work_node->child_num++;
        UpdateKey(work_node, work_node->keys[1]);
    }
    else
    {
        return -1;
    }
    return 0;
}
void MergeInternalNode(Node *work_node)
{
    Node *merge_from;
    if (work_node->next_node)
    {
        merge_from = work_node->next_node;
    }
    else if (work_node->pre_node)
    {
        merge_from = work_node;
        work_node = work_node->pre_node;
    }
    else
    {
        merge_from=work_node->parent_ptr;
        // MergeParentNode(work_node);
        work_node->next_node = merge_from->next_node;
        if (merge_from->next_node)
        {
            merge_from->next_node->pre_node = work_node;
        }
        work_node->parent_ptr=merge_from->parent_ptr;        
        free(merge_from);
        return;
    }
    for (int i = 0; i < merge_from->child_num; i++)
    {
        work_node->keys[work_node->child_num + i] = merge_from->keys[i];
        work_node->child_node_ptr[work_node->child_num + i] = merge_from->child_node_ptr[i];
        work_node->child_node_ptr[work_node->child_num+i]->parent_ptr=work_node;
    }
    work_node->next_node = merge_from->next_node;
    if (merge_from->next_node)
    {

        merge_from->next_node->pre_node = work_node;
    }
    work_node->child_num += merge_from->child_num;
    if(merge_from->parent_ptr)
    {
        DeleteInternalNodeOneItem(merge_from->parent_ptr, merge_from->keys[0]);
    }
    free(merge_from);

    return;
}
// void MergeParentNode(Node *work_node)
// {
//     Node *parent_node=work_node->parent_ptr;
//     for(int i=0;i<work_node->child_num,i++)
//     {
//         parent_node->keys=work_node->keys[i];

//     }
// }
void MergeLeafNode(Node *work_node)
{
    Node *merge_from;
    if (work_node->next_node)
    {
        merge_from = work_node->next_node;
    }
    else if (work_node->pre_node)
    {
        merge_from = work_node;
        work_node = work_node->pre_node;
    }
    else
    {

        return;
    }
    for (int i = 0; i < merge_from->child_num; i++)
    {
        work_node->keys[work_node->child_num + i] = merge_from->keys[i];
        work_node->values[work_node->child_num + i] = merge_from->values[i];
    }
    work_node->next_node = merge_from->next_node;
    if (merge_from->next_node)
    {
        merge_from->next_node->pre_node = work_node;
    }
    work_node->child_num += merge_from->child_num;
    if(merge_from->parent_ptr)
    {
        DeleteInternalNodeOneItem(merge_from->parent_ptr, merge_from->keys[0]);
    }

    free(merge_from);

    return;
}

void SplitInternalNode(Node *work_node)
{
    Node *new_internal_node = AllocNode(false);
    Node *new_parent_node;
    index_t min_key_in_new_node;
    for (int i = 0; i < MIN_CACHE_NUM; i++)
    {
        new_internal_node->keys[i] = work_node->keys[work_node->child_num - MIN_CACHE_NUM + i ];
        new_internal_node->child_node_ptr[i] = work_node->child_node_ptr[work_node->child_num - MIN_CACHE_NUM + i ];
        new_internal_node->child_node_ptr[i]->parent_ptr = new_internal_node;
    }
    min_key_in_new_node = new_internal_node->keys[0];
    work_node->child_num = MIN_CACHE_NUM;
    new_internal_node->child_num = MIN_CACHE_NUM;
    if (work_node->next_node)
        work_node->next_node->pre_node = new_internal_node;
    new_internal_node->pre_node = work_node;
    new_internal_node->next_node = work_node->next_node;
    work_node->next_node = new_internal_node;

    if (work_node->parent_ptr == NULL)
    {
        new_parent_node = AllocNode(false);
        new_parent_node->child_node_ptr[0] = work_node;
        new_parent_node->keys[0] = work_node->keys[0];
        new_parent_node->child_num = 1;

        work_node->parent_ptr = new_parent_node;
        // new_internal_node->parent_ptr=new_parent_node;
        b_plus_tree.root_node = new_parent_node;
        // first_leaf_node=work_node;

        // work_node->leaf=true;
    }
    InsertInternalNode(work_node->parent_ptr, min_key_in_new_node, new_internal_node);
    if (work_node->parent_ptr->child_num == 2 * MIN_CACHE_NUM)
    {
        // new_parent_node = AllocNode(false);
        SplitInternalNode(work_node->parent_ptr);
        // 记得把新节点所有子结点的父节点修改为新的
        // if (min_key_in_new_node > work_node->parent_ptr->keys[work_node->parent_ptr->child_num - 1])
        // {
            // new_internal_node->parent_ptr = work_node->parent_ptr->next_node;
        // }
        // else
        // {
        //     new_internal_node->parent_ptr = work_node->parent_ptr;
        // }
    }
}

void SplitLeafNode(Node *work_node)
{
    Node *new_leaf_node = AllocNode(true);
    Node *new_parent_node;
    index_t min_key_in_new_node;
    for (int i = 0; i < MIN_CACHE_NUM; i++)
    {
        new_leaf_node->keys[i] = work_node->keys[work_node->child_num - MIN_CACHE_NUM + i ];
        new_leaf_node->values[i] = work_node->values[work_node->child_num - MIN_CACHE_NUM + i ];
    }
    min_key_in_new_node = new_leaf_node->keys[0];
    work_node->child_num = MIN_CACHE_NUM;
    new_leaf_node->child_num = MIN_CACHE_NUM;

    if (work_node->next_node)
        work_node->next_node->pre_node = new_leaf_node;
    new_leaf_node->pre_node = work_node;
    new_leaf_node->next_node = work_node->next_node;
    work_node->next_node = new_leaf_node;
    if (work_node->parent_ptr == NULL)
    {
        new_parent_node = AllocNode(false);
        new_parent_node->child_node_ptr[0] = work_node;
        new_parent_node->keys[0] = work_node->keys[0];
        new_parent_node->child_num = 1;

        work_node->parent_ptr = new_parent_node;
        // new_leaf_node->parent_ptr=new_parent_node;
        b_plus_tree.root_node = new_parent_node;
        // first_leaf_node=work_node;

        work_node->leaf = true;
    }
    InsertInternalNode(work_node->parent_ptr, min_key_in_new_node, new_leaf_node);
    if (work_node->parent_ptr->child_num == 2 * MIN_CACHE_NUM)
    {
        // new_parent_node = AllocNode(false);
        SplitInternalNode(work_node->parent_ptr);
        // 记得把新节点所有子结点的父节点修改为新的
        // if (min_key_in_new_node > work_node->parent_ptr->keys[work_node->parent_ptr->child_num - 1])
        // {
            // new_leaf_node->parent_ptr = work_node->parent_ptr->next_node;
        // }
        // else
        // {
        //     new_leaf_node->parent_ptr = work_node->parent_ptr;
        // }
    }
}

void Insert(struct b_plus_tree *b_plus_tree, index_t key, Value value)
{
    Node *work_node = b_plus_tree->root_node;
    while (!work_node->leaf)
    {
        int i = SearchPos(work_node, key, true);
        if (i == LESS_THAN_MIN)
        {
            i = 0;
        }
        // int i = SearchPos(work_node, key, false);
        work_node = work_node->child_node_ptr[i];
        // b_plus_tree->leaf_nums++;
    }
    if (work_node->leaf)
    {
        InsertLeafNode(work_node, key, value);
        if (work_node->child_num == MIN_CACHE_NUM * 2)
        {
            SplitLeafNode(work_node);
            // if (key > work_node->keys[work_node->child_num - 1])
            // {
            //     work_node = work_node->next_node;
            // }
        }
        b_plus_tree->leaf_nums++;
    }
    return;
}
int SearchPos(Node *work_node, index_t key, bool is_search)
{
    if (key > work_node->keys[work_node->child_num - 1])
    {
        if (is_search)
        {
            return work_node->child_num - 1;
        }
        else
        {
            return work_node->child_num;
        }
    }
    for (int i = work_node->child_num - 1; i >= 0; i--)
    {
        if (key < work_node->keys[i])
        {
            continue;
        }

        else if (key == work_node->keys[i] && work_node->leaf)
        {
            return -1 * (i + 1);
        }
        else
        {
            if (is_search)
            {
                return i;
            }
            else
            {
                return i + 1;
            }
        }
    }
    if (is_search)
    {
            printf("less than min000000000000000000000000000\n");
            printf("%u\n",work_node->keys[0]);
            printf("%d\n",work_node->child_num);

        return LESS_THAN_MIN;
    }
    else
    {
        return 0;
    }
}

void InsertLeafNode(Node *work_node, index_t key, Value value)
{
    int i;
    if (work_node->child_num == 0)
    {
        work_node->keys[0] = key;
        work_node->values[0] = value;
        work_node->child_num++;
        return;
    }
    int insert_pos = SearchPos(work_node, key, false);
    if (insert_pos < 0)
    {
        work_node->values[-1 * (insert_pos + 1)] = value;
        b_plus_tree.leaf_nums--;
        return;
    }
    for (int i = work_node->child_num - 1; i >= insert_pos; i--)
    {
        work_node->keys[i + 1] = work_node->keys[i];
        work_node->values[i + 1] = work_node->values[i];
    }
    work_node->keys[insert_pos] = key;
    work_node->values[insert_pos] = value;
    work_node->child_num++;
    if (insert_pos == 0 && work_node->parent_ptr)
    {
        UpdateKey(work_node, work_node->keys[1]);
    }
    return;
}
void InsertInternalNode(Node *work_node, index_t key, Node *new_node)
{
    int i;
    if (work_node->child_num == 0)
    {
        work_node->keys[0] = key;
        work_node->child_node_ptr[0] = new_node;
        work_node->child_num++;
        return;
    }
    int insert_pos = SearchPos(work_node, key, false);
    if (insert_pos == LESS_THAN_MIN)
    {
        insert_pos = 0;
    }
    for (int i = work_node->child_num - 1; i >= insert_pos; i--)
    {
        work_node->keys[i + 1] = work_node->keys[i];
        work_node->child_node_ptr[i + 1] = work_node->child_node_ptr[i];
    }
    work_node->keys[insert_pos] = key;
    work_node->child_node_ptr[insert_pos] = new_node;

    new_node->parent_ptr = work_node;

    work_node->child_num++;
    if (insert_pos == 0 && work_node->parent_ptr)
    {
        UpdateKey(work_node, work_node->keys[1]);
    }
}
Value *Search(index_t key)
{
    Node *work_node = b_plus_tree.root_node;
    while (!work_node->leaf)
    {
        int i = SearchPos(work_node, key, true);
        if (i == LESS_THAN_MIN)
        {
            return NULL;
        }
        if (key >= work_node->keys[i])
        {
            work_node = work_node->child_node_ptr[i];
        }
    }
    int i = SearchPos(work_node, key, true);
    if (i == LESS_THAN_MIN)
    {
        return NULL;
    }
    if (i < 0)
    {
        return &(work_node->values[-1 * (i + 1)]);
    }
    else
    {
        return NULL;
    }
}
void UpdateKey(Node *work_node, index_t old_key)
{
    if (!work_node->parent_ptr)
    {
        return;
    }
    for (int i = 0; i < work_node->parent_ptr->child_num; i++)
    {
        if (work_node->parent_ptr->keys[i] == old_key)
        {
            work_node->parent_ptr->keys[i] = work_node->keys[0];
            if (i == 0)
            {
                UpdateKey(work_node->parent_ptr, old_key);
            }
            return;
        }
    }
}
struct b_plus_tree BPTreeCreate()
{
    //  struct b_plus_tree b_plus_tree;
    b_plus_tree.node_nums = 0;
    b_plus_tree.root_node = AllocNode(true);

    b_plus_tree.root_node->child_num = 0;
    first_leaf_node = b_plus_tree.root_node;
    return b_plus_tree;
}
Node *AllocNode(bool isLeaf)
{
    Node *new_node = malloc(sizeof(Node));
    // printf("***********************%d\n",b_plus_tree.node_nums);
    new_node->leaf = isLeaf;
    new_node->node_num = b_plus_tree.node_nums;
    b_plus_tree.node_nums++;
    return new_node;
}