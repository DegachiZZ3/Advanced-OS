#include <linux/module.h>       // Core header for loading modules
#include <linux/kernel.h>       // For printk()
#include <linux/init.h>         // For macros like __init and __exit

// The function to be called when the module is loaded
static int __init hello_world_init(void) {
    printk(KERN_INFO "Hello, World! Kernel module loaded.\n");
    return 0; // Return 0 means successful loading
}

// The function to be called when the module is unloaded
static void __exit hello_world_exit(void) {
    printk(KERN_INFO "Goodbye, World! Kernel module unloaded.\n");
}

// These macros define the init and exit functions
module_init(hello_world_init);
module_exit(hello_world_exit);

// Module information
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple Hello World Kernel Module");
