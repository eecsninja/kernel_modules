#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_DESCRIPTION("Hello world module");

static int hello_init(void) {
  printk("Hello world\n");
  return 0;
}

static void hello_cleanup(void) {
  printk("Goodbye\n");
}

module_init(hello_init);
module_exit(hello_cleanup);
