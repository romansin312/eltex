#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xd272d446, "__fentry__" },
	{ 0xf46d5bf3, "mutex_lock" },
	{ 0x437e81c7, "simple_read_from_buffer" },
	{ 0xf46d5bf3, "mutex_unlock" },
	{ 0xd272d446, "__x86_return_thunk" },
	{ 0xd61881e5, "simple_write_to_buffer" },
	{ 0xe8213e80, "_printk" },
	{ 0x11169e9e, "__register_chrdev" },
	{ 0x52b15b3b, "__unregister_chrdev" },
	{ 0xba157484, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0xd272d446,
	0xf46d5bf3,
	0x437e81c7,
	0xf46d5bf3,
	0xd272d446,
	0xd61881e5,
	0xe8213e80,
	0x11169e9e,
	0x52b15b3b,
	0xba157484,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"__fentry__\0"
	"mutex_lock\0"
	"simple_read_from_buffer\0"
	"mutex_unlock\0"
	"__x86_return_thunk\0"
	"simple_write_to_buffer\0"
	"_printk\0"
	"__register_chrdev\0"
	"__unregister_chrdev\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "5E8CB2F935158B67BE9FCE5");
