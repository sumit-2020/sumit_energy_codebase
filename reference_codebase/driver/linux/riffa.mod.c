#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x1fc32c62, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x6bc3fbc0, __VMLINUX_SYMBOL_STR(__unregister_chrdev) },
	{ 0x6a4bf290, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0xd2b09ce5, __VMLINUX_SYMBOL_STR(__kmalloc) },
	{ 0xc897c382, __VMLINUX_SYMBOL_STR(sg_init_table) },
	{ 0xf5893abf, __VMLINUX_SYMBOL_STR(up_read) },
	{ 0x5fe56825, __VMLINUX_SYMBOL_STR(dev_set_drvdata) },
	{ 0xc8b57c27, __VMLINUX_SYMBOL_STR(autoremove_wake_function) },
	{ 0x1d7c2a5c, __VMLINUX_SYMBOL_STR(dma_set_mask) },
	{ 0x77019e6d, __VMLINUX_SYMBOL_STR(pci_disable_device) },
	{ 0x900c670c, __VMLINUX_SYMBOL_STR(pcie_capability_read_dword) },
	{ 0x4bb0f1c6, __VMLINUX_SYMBOL_STR(device_destroy) },
	{ 0x4e317bdf, __VMLINUX_SYMBOL_STR(__register_chrdev) },
	{ 0x3fec048f, __VMLINUX_SYMBOL_STR(sg_next) },
	{ 0xa22b7df7, __VMLINUX_SYMBOL_STR(x86_dma_fallback_dev) },
	{ 0x2708194f, __VMLINUX_SYMBOL_STR(pci_release_regions) },
	{ 0x57a6ccd0, __VMLINUX_SYMBOL_STR(down_read) },
	{ 0xf432dd3d, __VMLINUX_SYMBOL_STR(__init_waitqueue_head) },
	{ 0x4f8b5ddb, __VMLINUX_SYMBOL_STR(_copy_to_user) },
	{ 0x905e1518, __VMLINUX_SYMBOL_STR(pci_set_master) },
	{ 0x6a39e9dd, __VMLINUX_SYMBOL_STR(current_task) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0xe27972cf, __VMLINUX_SYMBOL_STR(device_create) },
	{ 0x2072ee9b, __VMLINUX_SYMBOL_STR(request_threaded_irq) },
	{ 0x42c8de35, __VMLINUX_SYMBOL_STR(ioremap_nocache) },
	{ 0x28401d26, __VMLINUX_SYMBOL_STR(get_user_pages) },
	{        0, __VMLINUX_SYMBOL_STR(schedule_timeout) },
	{ 0xbdfb6dbb, __VMLINUX_SYMBOL_STR(__fentry__) },
	{ 0x9ecce67a, __VMLINUX_SYMBOL_STR(pci_unregister_driver) },
	{ 0x1a236d64, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0xcf21d241, __VMLINUX_SYMBOL_STR(__wake_up) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0x4423e05e, __VMLINUX_SYMBOL_STR(pci_request_regions) },
	{ 0x5c8b5ce8, __VMLINUX_SYMBOL_STR(prepare_to_wait) },
	{ 0xa6c0e1a0, __VMLINUX_SYMBOL_STR(pci_disable_msi) },
	{ 0x203b1c2, __VMLINUX_SYMBOL_STR(dma_supported) },
	{ 0xedc03953, __VMLINUX_SYMBOL_STR(iounmap) },
	{ 0xa348685e, __VMLINUX_SYMBOL_STR(__pci_register_driver) },
	{ 0xeaaebe8a, __VMLINUX_SYMBOL_STR(put_page) },
	{ 0xfd1ccd8a, __VMLINUX_SYMBOL_STR(class_destroy) },
	{ 0xfa66f77c, __VMLINUX_SYMBOL_STR(finish_wait) },
	{ 0x5bd232a3, __VMLINUX_SYMBOL_STR(pcie_capability_write_dword) },
	{ 0x28318305, __VMLINUX_SYMBOL_STR(snprintf) },
	{ 0xf1a7adf6, __VMLINUX_SYMBOL_STR(pci_enable_msi_block) },
	{ 0x8cd3f8d2, __VMLINUX_SYMBOL_STR(pci_enable_device) },
	{ 0x4f6b400b, __VMLINUX_SYMBOL_STR(_copy_from_user) },
	{ 0xf2d0cb22, __VMLINUX_SYMBOL_STR(__class_create) },
	{ 0xb749ae89, __VMLINUX_SYMBOL_STR(dev_get_drvdata) },
	{ 0x3007e10b, __VMLINUX_SYMBOL_STR(dma_ops) },
	{ 0xf20dabd8, __VMLINUX_SYMBOL_STR(free_irq) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("pci:v000010EEd*sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00001172d*sv*sd*bc*sc*i*");

MODULE_INFO(srcversion, "68F53A2620B185105878C40");
