#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/list.h>

// Define the node structure
struct my_node {
    int data;
    struct list_head list; // Kernel's list structure
};

// Head of the linked list
static LIST_HEAD(my_list_head);

// Module initialization
static int __init linked_list_init(void)
{
    struct my_node *node;
    int i;

    printk(KERN_INFO "Linked List Module: Initializing\n");

    // Adding nodes to the list
    for (i = 0; i < 5; i++) {
        node = kmalloc(sizeof(*node), GFP_KERNEL);
        if (!node) {
            printk(KERN_ERR "Linked List Module: Memory allocation failed\n");
            return -ENOMEM;
        }

        node->data = i + 1; // Example data
        list_add_tail(&node->list, &my_list_head);
        printk(KERN_INFO "Linked List Module: Added node with data = %d\n", node->data);
    }

    // Traversing the list
    printk(KERN_INFO "Linked List Module: Traversing the list\n");
    list_for_each_entry(node, &my_list_head, list) {
        printk(KERN_INFO "Node data: %d\n", node->data);
    }

    return 0;
}

// Module cleanup
static void __exit linked_list_exit(void)
{
    struct my_node *node, *tmp;

    printk(KERN_INFO "Linked List Module: Cleaning up\n");

    // Deleting the list and freeing memory
    list_for_each_entry_safe(node, tmp, &my_list_head, list) {
        printk(KERN_INFO "Freeing node with data = %d\n", node->data);
        list_del(&node->list); // Remove from the list
        kfree(node);           // Free memory
    }

    printk(KERN_INFO "Linked List Module: Cleanup complete\n");
}

module_init(linked_list_init);
module_exit(linked_list_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A Linux Kernel Module for Linked List Management");
