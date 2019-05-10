#ifndef NET_CODERODDE_BINARY_TREE_H
#define NET_CODERODDE_BINARY_TREE_H

#include <malloc.h>

typedef struct binary_tree_node_t {
    int datum;
    struct binary_tree_node_t* left;
    struct binary_tree_node_t* right;
} binary_tree_node_t;

typedef struct binary_tree_t {
    binary_tree_node_t* root;
} binary_tree_t;

int binary_tree_t_init(binary_tree_t* tree);
int binary_tree_t_insert(binary_tree_t* tree, int datum);
void binary_tree_t_print(binary_tree_t* tree);
void binary_tree_t_free(binary_tree_t* tree);

int binary_tree_t_init(binary_tree_t* tree)
{
    if (!tree)
    {
        return 0;
    }

    tree->root = NULL;
    return 1;
}

int binary_tree_t_insert(binary_tree_t* tree, int datum)
{
    binary_tree_node_t* new_node;
    binary_tree_node_t* curr_node;
    binary_tree_node_t* prev_node;

    if (!tree)
    {
        return 0;
    }

    new_node = (binary_tree_node_t*) malloc(sizeof(binary_tree_node_t));

    if (!new_node)
    {
        return 0;
    }

    new_node->left  = NULL;
    new_node->right = NULL;
    new_node->datum = datum;

    if (!tree->root)
    {
        tree->root = new_node;
    }
    else
    {
        prev_node = NULL;
        curr_node = tree->root;

        while (curr_node)
        {
            if (datum < curr_node->datum)
            {
                prev_node = curr_node;
                curr_node = curr_node->left;
            }
            else if (datum > curr_node->datum)
            {
                prev_node = curr_node;
                curr_node = curr_node->right;
            }
            else
            {
                /* datum already in the tree */
                return 1;
            }
        }

        if (datum < prev_node->datum)
        {
            prev_node->left = new_node;
        }
        else
        {
            prev_node->right = new_node;
        }
    }

    return 1;
}

static void binary_tree_t_print_impl(binary_tree_node_t* node)
{
    if (!node)
    {
        return;
    }

    binary_tree_t_print_impl(node->left);
    printf("%d ", node->datum);
    binary_tree_t_print_impl(node->right);
}

void binary_tree_t_print(binary_tree_t* tree)
{
    if (!tree)
    {
        printf("null tree");
        return;
    }

    binary_tree_t_print_impl(tree->root);
}

static void binary_tree_t_free_impl(binary_tree_node_t* node)
{
    if (!node)
    {
        return;
    }

    binary_tree_t_free_impl(node->left);
    binary_tree_t_free_impl(node->right);
    free(node);
}

void binary_tree_t_free(binary_tree_t* tree)
{
    if (!tree)
    {
        return;
    }

    binary_tree_t_free_impl(tree->root);
}
#endif /* NET_CODERODDE_BINARY_TREE_H */
