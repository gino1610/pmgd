#include <iostream>
#include <string.h>

#include "jarvis.h"
#include "../src/List.h"
#include "../src/os.h"
#include "../src/allocator.h"
#include "../src/AvlTree.h"
#include "../util/util.h"

using namespace Jarvis;
using namespace std;

#define REGION_SIZE 4096
#define NUM_FIXED_ALLOCATORS 5

static constexpr AllocatorInfo default_allocators[] = {
    { 0, REGION_SIZE, 16 },
    { 1*REGION_SIZE, REGION_SIZE, 32 },
    { 2*REGION_SIZE, REGION_SIZE, 64 },
    { 3*REGION_SIZE, REGION_SIZE, 128 },
    { 4*REGION_SIZE, REGION_SIZE, 256 },
};

void print_recursive(AvlTree<int> &tree, AvlTree<int>::AvlTreeNode *node,
                     char side, int rec_depth)
{
    if (node == NULL)
        return;
    for (int i = 0; i < rec_depth; ++i)
        cout << " ";
    cout << side << ":" << *tree.data(node) << "[" << tree.height(node) << "]\n";
    print_recursive(tree, tree.left(node), 'L', rec_depth + 1);
    print_recursive(tree, tree.right(node), 'R', rec_depth + 1);
}

void print(AvlTree<int> &tree)
{
    print_recursive(tree, tree.begin(), 'C', 0);
    cout << endl;
}

void print_node(AvlTree<int> &tree, AvlTree<int>::AvlTreeNode *curr)
{
    if (curr == NULL) {
        cout << "Empty\n";
        return;
    }
    cout << "Data: " << *tree.data(curr) << "\n";
}

int main()
{
    cout << "AVLTree unit test\n\n";
    uint64_t start_addr;
    try {
        Graph db("avlgraph", Graph::Create);

        Transaction tx(db);
        // Need the allocator
        struct AllocatorInfo info1;
        struct AllocatorInfo info_arr[sizeof default_allocators];
        bool create1 = true;

        start_addr = 0x100000000;
        memcpy(info_arr, default_allocators, sizeof default_allocators);

        info1.offset = 0;
        info1.len = NUM_FIXED_ALLOCATORS * REGION_SIZE;
        info1.size = 0;

        os::MapRegion region1(".", "region1", start_addr, info1.len, create1, create1);
        Allocator allocator1(start_addr, info_arr, NUM_FIXED_ALLOCATORS, create1);
        
        AvlTree<int> tree;
        int insert_vals[] = {5, 5, 10, 15, 20, 25, 1, 4, 30, 35, 40, 45, 50,
            12, 17, 60, 70, 70, 55, 19};
        int num_inserted = 18 + 2;

        for (int i = 0; i < num_inserted; ++i)
            tree.add(insert_vals[i], allocator1);
        cout << "Num elements: " << tree.num_elems() << "\n";
        print(tree);

        int find_val = 66;
        print_node(tree, tree.find(find_val));
        find_val = 50;
        print_node(tree, tree.find(find_val));

        set<AvlTree<int>::AvlTreeNode *> r;
        int min = 10, max = 25;
        tree.find_range(r, min, max, true);
        cout << "Items in range: " << min << "-" << max << ", inclusive:\n";
        for (set<AvlTree<int>::AvlTreeNode *>::iterator it = r.begin(); it != r.end(); ++it)
            print_node(tree, *it);

        r.clear();
        min = 12; max = 60;
        tree.find_range(r, min, max, true);
        cout << "Items in range: " << min << "-" << max << ", inclusive:\n";
        for (set<AvlTree<int>::AvlTreeNode *>::iterator it = r.begin(); it != r.end(); ++it)
            print_node(tree, *it);

        r.clear();
        min = 55; max = 70;
        tree.find_range(r, min, max, true);
        cout << "Items in range: " << min << "-" << max << ", inclusive:\n";
        for (set<AvlTree<int>::AvlTreeNode *>::iterator it = r.begin(); it != r.end(); ++it)
            print_node(tree, *it);

        r.clear();
        min = 1; max = 70;
        tree.find_range(r, min, max, false);
        cout << "Items in range: " << min << "-" << max << ", inclusive:\n";
        for (set<AvlTree<int>::AvlTreeNode *>::iterator it = r.begin(); it != r.end(); ++it)
            print_node(tree, *it);

        cout << "Testing remove\n";
        int remove_vals[] = {55, 55, 60, 45, 40, 15, 17, 10};
        int num_removed = 8;
        for (int i = 0; i < num_removed; ++i)
            tree.remove(remove_vals[i], allocator1);
        cout << "Num elements: " << tree.num_elems() << "\n";
        print(tree);
        // Now just delete the remaining
        int array[] = {5, 20, 25, 12, 19, 1, 4, 30, 35, 50, 70};
        for (int i = 0; i < 11; ++i)
            tree.remove(array[i], allocator1);
        cout << "Num elements: " << tree.num_elems() << "\n";
        print(tree);
    }
    catch (Exception e) {
        print_exception(e);
    }
    return 0;
}

