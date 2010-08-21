#ifndef ARM_ARCH_BCM_VUART_H
#define ARM_ARCH_BCM_VUART_H



struct bvmvuart_platform_data {
	int (*init)(struct platform_device *pdev);
	void (*exit)(struct platform_device *pdev);
	unsigned int flags;
};





#endif /* ARM_ARCH_BCM_VUART_H */
