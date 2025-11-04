#ifndef BPTREE_HPP
#define BPTREE_HPP

#include <fstream>
#include <cstring>

template<class Key, class Value, int M = 100, int L = 100>
class BPTree {
private:
    static const int MAX_KEY = M;
    static const int MAX_DATA = L;
    static const int MIN_KEY = M / 2;
    static const int MIN_DATA = L / 2;

    struct Node {
        int size;
        bool is_leaf;
        int offset;
        int parent;
        int next;
        Key keys[MAX_KEY + 1];
        int children[MAX_KEY + 2];
        
        Node() : size(0), is_leaf(false), offset(-1), parent(-1), next(-1) {}
    };

    struct Leaf {
        int size;
        int offset;
        int parent;
        int next;
        Key keys[MAX_DATA + 1];
        Value values[MAX_DATA + 1];
        
        Leaf() : size(0), offset(-1), parent(-1), next(-1) {}
    };

    std::fstream file;
    std::string filename;
    int root_offset;
    int node_count;
    int leaf_count;
    bool is_root_leaf;

    void read_node(int offset, Node& node) {
        file.seekg(offset);
        file.read(reinterpret_cast<char*>(&node), sizeof(Node));
    }

    void write_node(int offset, const Node& node) {
        file.seekp(offset);
        file.write(reinterpret_cast<const char*>(&node), sizeof(Node));
    }

    void read_leaf(int offset, Leaf& leaf) {
        file.seekg(offset);
        file.read(reinterpret_cast<char*>(&leaf), sizeof(Leaf));
    }

    void write_leaf(int offset, const Leaf& leaf) {
        file.seekp(offset);
        file.write(reinterpret_cast<const char*>(&leaf), sizeof(Leaf));
    }

    int allocate_node() {
        int offset = sizeof(int) * 4 + node_count * sizeof(Node) + leaf_count * sizeof(Leaf);
        node_count++;
        return offset;
    }

    int allocate_leaf() {
        int offset = sizeof(int) * 4 + node_count * sizeof(Node) + leaf_count * sizeof(Leaf);
        leaf_count++;
        return offset;
    }

    void write_header() {
        file.seekp(0);
        file.write(reinterpret_cast<char*>(&root_offset), sizeof(int));
        file.write(reinterpret_cast<char*>(&node_count), sizeof(int));
        file.write(reinterpret_cast<char*>(&leaf_count), sizeof(int));
        int is_leaf = is_root_leaf ? 1 : 0;
        file.write(reinterpret_cast<char*>(&is_leaf), sizeof(int));
        file.flush();
    }

    void read_header() {
        file.seekg(0);
        file.read(reinterpret_cast<char*>(&root_offset), sizeof(int));
        file.read(reinterpret_cast<char*>(&node_count), sizeof(int));
        file.read(reinterpret_cast<char*>(&leaf_count), sizeof(int));
        int is_leaf;
        file.read(reinterpret_cast<char*>(&is_leaf), sizeof(int));
        is_root_leaf = (is_leaf == 1);
    }

    void split_leaf(Leaf& leaf, const Key& key, const Value& value, int leaf_offset) {
        Leaf new_leaf;
        new_leaf.offset = allocate_leaf();
        new_leaf.parent = leaf.parent;
        new_leaf.next = leaf.next;
        leaf.next = new_leaf.offset;

        Key temp_keys[MAX_DATA + 2];
        Value temp_values[MAX_DATA + 2];
        int pos = 0;
        for (int i = 0; i < leaf.size; i++) {
            if (pos == i && key < leaf.keys[i]) {
                temp_keys[pos] = key;
                temp_values[pos] = value;
                pos++;
            }
            temp_keys[pos] = leaf.keys[i];
            temp_values[pos] = leaf.values[i];
            pos++;
        }
        if (pos == leaf.size) {
            temp_keys[pos] = key;
            temp_values[pos] = value;
            pos++;
        }

        int mid = (pos + 1) / 2;
        leaf.size = mid;
        for (int i = 0; i < mid; i++) {
            leaf.keys[i] = temp_keys[i];
            leaf.values[i] = temp_values[i];
        }

        new_leaf.size = pos - mid;
        for (int i = 0; i < new_leaf.size; i++) {
            new_leaf.keys[i] = temp_keys[mid + i];
            new_leaf.values[i] = temp_values[mid + i];
        }

        write_leaf(leaf_offset, leaf);
        write_leaf(new_leaf.offset, new_leaf);

        insert_into_parent(leaf_offset, new_leaf.keys[0], new_leaf.offset, true);
    }

    void split_node(Node& node, const Key& key, int child_offset, int node_offset) {
        Node new_node;
        new_node.offset = allocate_node();
        new_node.parent = node.parent;
        new_node.is_leaf = false;

        Key temp_keys[MAX_KEY + 2];
        int temp_children[MAX_KEY + 3];
        int pos = 0;
        for (int i = 0; i < node.size; i++) {
            if (pos == i && key < node.keys[i]) {
                temp_keys[pos] = key;
                temp_children[pos + 1] = child_offset;
                pos++;
            }
            temp_keys[pos] = node.keys[i];
            temp_children[pos + 1] = node.children[i + 1];
            pos++;
        }
        if (pos == node.size) {
            temp_keys[pos] = key;
            temp_children[pos + 1] = child_offset;
            pos++;
        }
        temp_children[0] = node.children[0];

        int mid = pos / 2;
        node.size = mid;
        for (int i = 0; i < mid; i++) {
            node.keys[i] = temp_keys[i];
            node.children[i] = temp_children[i];
        }
        node.children[mid] = temp_children[mid];

        new_node.size = pos - mid - 1;
        for (int i = 0; i < new_node.size; i++) {
            new_node.keys[i] = temp_keys[mid + 1 + i];
            new_node.children[i] = temp_children[mid + 1 + i];
        }
        new_node.children[new_node.size] = temp_children[pos];

        write_node(node_offset, node);
        write_node(new_node.offset, new_node);

        insert_into_parent(node_offset, temp_keys[mid], new_node.offset, false);
    }

    void insert_into_parent(int left_offset, const Key& key, int right_offset, bool is_leaf_child) {
        if (is_root_leaf && is_leaf_child) {
            Node new_root;
            new_root.offset = allocate_node();
            new_root.is_leaf = false;
            new_root.parent = -1;
            new_root.size = 1;
            new_root.keys[0] = key;
            new_root.children[0] = left_offset;
            new_root.children[1] = right_offset;

            Leaf left, right;
            read_leaf(left_offset, left);
            read_leaf(right_offset, right);
            left.parent = new_root.offset;
            right.parent = new_root.offset;
            write_leaf(left_offset, left);
            write_leaf(right_offset, right);

            write_node(new_root.offset, new_root);
            root_offset = new_root.offset;
            is_root_leaf = false;
            write_header();
            return;
        }

        int parent_offset;
        if (is_leaf_child) {
            Leaf child;
            read_leaf(left_offset, child);
            parent_offset = child.parent;
        } else {
            Node child;
            read_node(left_offset, child);
            parent_offset = child.parent;
        }

        if (parent_offset == -1) {
            Node new_root;
            new_root.offset = allocate_node();
            new_root.is_leaf = false;
            new_root.parent = -1;
            new_root.size = 1;
            new_root.keys[0] = key;
            new_root.children[0] = left_offset;
            new_root.children[1] = right_offset;

            Node left, right;
            read_node(left_offset, left);
            read_node(right_offset, right);
            left.parent = new_root.offset;
            right.parent = new_root.offset;
            write_node(left_offset, left);
            write_node(right_offset, right);

            write_node(new_root.offset, new_root);
            root_offset = new_root.offset;
            write_header();
            return;
        }

        Node parent;
        read_node(parent_offset, parent);

        if (parent.size < MAX_KEY) {
            int pos = parent.size;
            while (pos > 0 && key < parent.keys[pos - 1]) {
                parent.keys[pos] = parent.keys[pos - 1];
                parent.children[pos + 1] = parent.children[pos];
                pos--;
            }
            parent.keys[pos] = key;
            parent.children[pos + 1] = right_offset;
            parent.size++;

            if (is_leaf_child) {
                Leaf right;
                read_leaf(right_offset, right);
                right.parent = parent_offset;
                write_leaf(right_offset, right);
            } else {
                Node right;
                read_node(right_offset, right);
                right.parent = parent_offset;
                write_node(right_offset, right);
            }

            write_node(parent_offset, parent);
        } else {
            split_node(parent, key, right_offset, parent_offset);
        }
    }

public:
    BPTree(const std::string& fname) : filename(fname) {
        file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
        if (!file.is_open()) {
            file.open(filename, std::ios::out | std::ios::binary);
            file.close();
            file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
            
            root_offset = sizeof(int) * 4;
            node_count = 0;
            leaf_count = 1;
            is_root_leaf = true;

            Leaf root_leaf;
            root_leaf.offset = root_offset;
            root_leaf.parent = -1;
            root_leaf.next = -1;
            root_leaf.size = 0;

            write_header();
            write_leaf(root_offset, root_leaf);
        } else {
            read_header();
        }
    }

    ~BPTree() {
        if (file.is_open()) {
            file.close();
        }
    }

    void insert(const Key& key, const Value& value) {
        if (is_root_leaf) {
            Leaf root;
            read_leaf(root_offset, root);

            if (root.size < MAX_DATA) {
                int pos = root.size;
                while (pos > 0 && key < root.keys[pos - 1]) {
                    root.keys[pos] = root.keys[pos - 1];
                    root.values[pos] = root.values[pos - 1];
                    pos--;
                }
                root.keys[pos] = key;
                root.values[pos] = value;
                root.size++;
                write_leaf(root_offset, root);
            } else {
                split_leaf(root, key, value, root_offset);
            }
        } else {
            int current = root_offset;
            Node node;
            
            while (true) {
                read_node(current, node);
                int pos = 0;
                while (pos < node.size && !(key < node.keys[pos])) {
                    pos++;
                }
                
                if (pos == 0) {
                    current = node.children[0];
                } else {
                    current = node.children[pos];
                }

                file.seekg(current);
                bool is_leaf;
                file.seekg(current + sizeof(int));
                file.read(reinterpret_cast<char*>(&is_leaf), sizeof(bool));
                
                if (is_leaf) {
                    break;
                }
            }

            Leaf leaf;
            read_leaf(current, leaf);

            if (leaf.size < MAX_DATA) {
                int pos = leaf.size;
                while (pos > 0 && key < leaf.keys[pos - 1]) {
                    leaf.keys[pos] = leaf.keys[pos - 1];
                    leaf.values[pos] = leaf.values[pos - 1];
                    pos--;
                }
                leaf.keys[pos] = key;
                leaf.values[pos] = value;
                leaf.size++;
                write_leaf(current, leaf);
            } else {
                split_leaf(leaf, key, value, current);
            }
        }
    }

    bool find(const Key& key, Value& value) {
        if (is_root_leaf) {
            Leaf root;
            read_leaf(root_offset, root);
            for (int i = 0; i < root.size; i++) {
                if (!(root.keys[i] < key) && !(key < root.keys[i])) {
                    value = root.values[i];
                    return true;
                }
            }
            return false;
        }

        int current = root_offset;
        Node node;
        
        while (true) {
            read_node(current, node);
            int pos = 0;
            while (pos < node.size && !(key < node.keys[pos])) {
                pos++;
            }
            
            if (pos == 0) {
                current = node.children[0];
            } else {
                current = node.children[pos];
            }

            file.seekg(current + sizeof(int));
            bool is_leaf;
            file.read(reinterpret_cast<char*>(&is_leaf), sizeof(bool));
            
            if (is_leaf) {
                break;
            }
        }

        Leaf leaf;
        read_leaf(current, leaf);
        for (int i = 0; i < leaf.size; i++) {
            if (!(leaf.keys[i] < key) && !(key < leaf.keys[i])) {
                value = leaf.values[i];
                return true;
            }
        }
        return false;
    }

    void clear() {
        file.close();
        file.open(filename, std::ios::out | std::ios::binary);
        file.close();
        file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
        
        root_offset = sizeof(int) * 4;
        node_count = 0;
        leaf_count = 1;
        is_root_leaf = true;

        Leaf root_leaf;
        root_leaf.offset = root_offset;
        root_leaf.parent = -1;
        root_leaf.next = -1;
        root_leaf.size = 0;

        write_header();
        write_leaf(root_offset, root_leaf);
    }
};

#endif

