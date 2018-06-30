#ifndef PERSISTENT_SET_LIBRARY_H
#define PERSISTENT_SET_LIBRARY_H

#include <cassert>  // assert
#include <iterator> // std::reverse_iterator
#include <utility>  // std::pair, std::swap
#include <memory>

template<typename T>
struct persistent_set {
    typedef T value_type;
    struct bNode;
    struct node;


    struct iterator;
    using const_iterator = iterator;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    const_iterator begin() const;

    const_iterator end() const;

    const_reverse_iterator rbegin() const;

    const_reverse_iterator rend() const;


    persistent_set();

    persistent_set(persistent_set const &);

    //persistent_set &operator=(persistent_set const &other);

    //~persistent_set();

    void clear();

    bool empty() const;


    void swap(persistent_set &other);

    iterator find(T const &value) const;

    std::pair<iterator, bool> insert(T const &value);

    void erase(iterator const &it);

    struct bNode {
        friend struct persistent_set;
        std::shared_ptr<bNode> left;
        std::shared_ptr<bNode> right;

        bNode();

        bNode(std::shared_ptr<bNode> const &left, std::shared_ptr<bNode> const &right)
                : left(left), right(right) {}

        T &get_value();

        bNode *min();

        bNode *max();

        bNode *next(bNode *root);

        bNode *prev(bNode *root);
    };

private:

    void tree_();

    std::shared_ptr<bNode> erase_impl(bNode *pos, bNode *pos2);

    std::shared_ptr<bNode> insert_impl(bNode *pos, T const &value, bNode *&result);

    std::shared_ptr<bNode> tree;

    size_t _size;
};


template<typename T>
struct persistent_set<T>::node : bNode {
    node(T const &value) : value(value) {}

    node(std::shared_ptr<bNode> const &left, std::shared_ptr<bNode> const &right, T const &value)
            : bNode(left, right), value(value) {}

private:
    friend struct bNode;
    T value;
};

template<typename T>
typename persistent_set<T>::const_iterator persistent_set<T>::begin() const {
    if (!tree || !tree->left) {
        return end();
    }
    return const_iterator(tree.get()->min(), tree.get());
}


template<typename T>
typename persistent_set<T>::const_iterator persistent_set<T>::end() const {
    return const_iterator(tree.get(), tree.get());
}

template<typename T>
typename persistent_set<T>::const_reverse_iterator persistent_set<T>::rbegin() const {
    return const_reverse_iterator(end());
}

template<typename T>
typename persistent_set<T>::const_reverse_iterator persistent_set<T>::rend() const {
    return const_reverse_iterator(begin());
}

template<typename T>
struct persistent_set<T>::iterator {
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = T const;
    using pointer = T const *;
    using reference = T const &;

    iterator() = default;

    reference operator*() const;

    pointer operator->() const;

    iterator &operator++();

    iterator operator++(int);

    iterator &operator--();

    iterator operator--(int);

    friend bool operator==(iterator const &a, iterator const &b) {
        return a._node == b._node;
    }

    friend bool operator!=(iterator const &a, iterator const &b) {
        return a._node != b._node;
    }

private:
    friend struct persistent_set;
    bNode *_node;
    bNode *root;

    explicit iterator(bNode *_node, bNode *root) : _node(_node), root(root) {}
};

template<typename T>
persistent_set<T>::persistent_set() {
    tree = nullptr;
    _size = 0;
}

template<typename T>
void persistent_set<T>::swap(persistent_set &other) {
    std::swap(tree, other.tree);
    std::swap(_size, other._size);
}

template<typename T>
typename persistent_set<T>::iterator persistent_set<T>::find(T const &value) const {
    if (!tree) {
        return end();
    } else {
        auto cur = tree->left.get();
        for (;;) {
            if (cur == nullptr) {
                return end();
            } else {
                if (cur->get_value() > value) {
                    cur = cur->left.get();
                } else if (cur->get_value() < value) {
                    cur = cur->right.get();
                } else {
                    return iterator(cur, tree.get());
                }
            }
        }
    }
}

template<typename T>
std::pair<typename persistent_set<T>::iterator, bool> persistent_set<T>::insert(T const &value) {
    tree_();
    auto res = find(value);
    if (res != end()) {
        return {res, false};
    } else {
        bNode *result = nullptr;
        auto tmp_tree = std::make_shared<persistent_set<T>::bNode>();
        tmp_tree->left = insert_impl(tree->left.get(), value, result);

        tree = tmp_tree;
        _size++;

        return {persistent_set<T>::iterator(result, tree.get()), true};
    }
}

template<typename T>
void persistent_set<T>::erase(const persistent_set<T>::iterator &it) {
    if (tree && tree->left) {
        auto tmp_tree = std::make_shared<persistent_set<T>::bNode>();
        tmp_tree->left = erase_impl(tree->left.get(), it._node);

        tree = tmp_tree;
        _size--;
    }
}

template<typename T>
typename persistent_set<T>::bNode *persistent_set<T>::bNode::next(persistent_set::bNode *root) {
    if (right) {
        return right->min();

    } else {

        auto cur = root->left;
        auto result = root;

        for (;;) {
            if (cur->get_value() < get_value()) {
                cur = cur->right;
            } else if (cur->get_value() > get_value()) {
                result = cur.get();
                cur = cur->left;
            } else {
                return result;
            }
        }

    }
}

template<typename T>
typename persistent_set<T>::bNode *persistent_set<T>::bNode::prev(persistent_set::bNode *root) {
    if (left) {
        return left->max();

    } else {

        auto cur = root->left;
        auto result = root;

        for (;;) {
            if (cur->get_value() < get_value()) {
                result = cur.get();
                cur = cur->right;
            } else if (cur->get_value() > get_value()) {
                cur = cur->left;
            } else {
                return result;
            }

        }
    }
}

template<typename T>
std::shared_ptr<typename persistent_set<T>::bNode>
persistent_set<T>::insert_impl(persistent_set::bNode *pos, const T &value, persistent_set::bNode *&result) {
    if (!pos) {
        auto _new = std::make_shared<typename persistent_set<T>::node>(value);
        result = _new.get();
        return _new;


    } else if (pos->get_value() < value) {

        return std::make_shared<typename persistent_set<T>::node>
                (pos->left, insert_impl(pos->right.get(), value, result), pos->get_value());

    } else {

        return std::make_shared<typename persistent_set<T>::node>
                (insert_impl(pos->left.get(), value, result), pos->right, pos->get_value());
    }
}

template<typename T>
std::shared_ptr<typename persistent_set<T>::bNode>
persistent_set<T>::erase_impl(persistent_set::bNode *pos, persistent_set::bNode *pos2) {
    if (pos == pos2) {

        if (!pos2->right) {
            return pos2->left;

        } else if (!pos2->left) {
            return pos2->right;

        } else {

            bNode *minimum = pos->right->min();
            return std::make_shared<typename persistent_set<T>::node>
                    (pos->left, erase_impl(pos->right.get(), minimum), minimum->get_value());
        }

    } else if (pos->get_value() < pos2->get_value()) {
        return std::make_shared<typename persistent_set<T>::node>
                (pos->left, erase_impl(pos->right.get(), pos2), pos->get_value());
    } else {
        return std::make_shared<typename persistent_set<T>::node>
                (erase_impl(pos->left.get(), pos2), pos->right, pos->get_value());
    }
}

template<typename T>
void persistent_set<T>::tree_() {
    if (!tree)
        tree = std::make_shared<bNode>();
}

template<typename T>
persistent_set<T>::persistent_set(persistent_set const &other) {
    tree = other.tree;
    _size = other._size;
}

template<typename T>
bool persistent_set<T>::empty() const {
    return _size == 0;
}

template<typename T>
void persistent_set<T>::clear() {
    tree = nullptr;
    _size = 0;
}

template<typename T>
void swap(persistent_set<T> &a, persistent_set<T> &b) {
    a.swap(b);
}

template<typename T>
T &persistent_set<T>::bNode::get_value() {
    return static_cast<node *>(this)->value;
}

template<typename T>
persistent_set<T>::bNode::bNode() {
    left = nullptr;
}

template<typename T>
typename persistent_set<T>::bNode *persistent_set<T>::bNode::min() {
    auto cur = this;
    while (cur->left != nullptr) {
        cur = cur->left.get();
    }
    return cur;
}

template<typename T>
typename persistent_set<T>::bNode *persistent_set<T>::bNode::max() {
    auto cur = this;
    while (cur->right != nullptr) {
        cur = cur->right.get();
    }
    return cur;
}

template<typename T>
typename persistent_set<T>::iterator &persistent_set<T>::iterator::operator++() {
    _node = _node->next(root);
    return *this;
}

template<typename T>
typename persistent_set<T>::iterator persistent_set<T>::iterator::operator++(int) {
    iterator copy = *this;
    ++*this;
    return copy;
}

template<typename T>
typename persistent_set<T>::iterator &persistent_set<T>::iterator::operator--() {
    _node = _node->prev(root);
    return *this;
}

template<typename T>
typename persistent_set<T>::iterator persistent_set<T>::iterator::operator--(int) {
    iterator copy = *this;
    --*this;
    return copy;
}

template<typename T>
typename persistent_set<T>::iterator::reference &persistent_set<T>::iterator::operator*() const {
    return _node->get_value();
}

template<typename T>
typename persistent_set<T>::iterator::pointer persistent_set<T>::iterator::operator->() const {
    return &_node->get_value();
}

#endif