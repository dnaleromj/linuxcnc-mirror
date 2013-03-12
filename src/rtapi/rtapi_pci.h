#ifndef RTAPI_PCI_H
#define RTAPI_PCI_H

#include "config.h"

/***********************************************************************
*                        PCI DEVICE SUPPORT                            *
************************************************************************/
#if defined(BUILD_SYS_USER_DSO)

/*---------------------------------------------------------------------**
** Structures and defines that should be pulled in from <linux/pci.h>  **
** ...except we're not compiling kernel code.  :(                      **
**---------------------------------------------------------------------*/

struct pci_device_id {
    __u32 vendor, device;           /* Vendor and device ID or PCI_ANY_ID*/
    __u32 subvendor, subdevice;     /* Subsystem ID's or PCI_ANY_ID */
    __u32 class, class_mask;        /* (class,subclass,prog-if) triplet */
};

/** FIXME: Support more thane one board! **/
struct pci_resource {
    void            *start;         /* Details read from device/resource file */
    void            *end;
    unsigned long   flags;
    int             fd;             /* sysfs_pci device/resourceN file descriptor */
    void            *mmio;          /* The mmap address (if mapped) */
};

struct pci_dev {
    char            dev_name[32];   /* Device name (0000:03:00.0) */
    char            sys_path[256];  /* Path to device (/sys/device/... */
    unsigned short  vendor;
    unsigned short  device;
    unsigned short  subsystem_vendor;
    unsigned short  subsystem_device;
    unsigned int    class;          /* 3 bytes: (base,sub,prog-if) */
    struct pci_resource   
                    resource[6];    /* Device BARs */    
    void *driver_data;              /* Data private to the driver */
};

typedef struct pm_message {
    int event;
} pm_message_t;

struct pci_driver {
//    struct list_head node;
    char *name;
    const struct pci_device_id *id_table;   /* must be non-NULL for probe to be called */
    int  (*probe)  (struct pci_dev *dev, const struct pci_device_id *id);   /* New device inserted */
    void (*remove) (struct pci_dev *dev);   /* Device removed (NULL if not a hot-plug capable driver) */
    int  (*suspend) (struct pci_dev *dev, pm_message_t state);      /* Device suspended */
    int  (*suspend_late) (struct pci_dev *dev, pm_message_t state);
    int  (*resume_early) (struct pci_dev *dev);
    int  (*resume) (struct pci_dev *dev);                   /* Device woken up */
    void (*shutdown) (struct pci_dev *dev);
//    struct pci_error_handlers *err_handler;
//    struct device_driver    driver;
//    struct pci_dynids dynids;
};


int pci_register_driver(struct pci_driver *driver);
void pci_unregister_driver(struct pci_driver *driver);
int pci_enable_device(struct pci_dev *dev);
int pci_disable_device(struct pci_dev *dev);

int rtapi_assure_module_loaded(const char *module);

#define pci_resource_start(dev, bar)    ((dev)->resource[(bar)].start)
#define pci_resource_end(dev, bar)      ((dev)->resource[(bar)].end)
#define pci_resource_flags(dev, bar)    ((dev)->resource[(bar)].flags)
#define pci_resource_len(dev, bar) \
        ((pci_resource_start((dev), (bar)) == 0 &&    \
          pci_resource_end((dev), (bar)) ==           \
          pci_resource_start((dev), (bar))) ? 0 :     \
                                                            \
         (pci_resource_end((dev), (bar)) -            \
          pci_resource_start((dev), (bar)) + 1))

void __iomem *pci_ioremap_bar(struct pci_dev *pdev, int bar);
inline void iounmap(volatile void __iomem *addr);

static inline const char *pci_name(const struct pci_dev *pdev)
{
    return pdev->dev_name;
}

static inline void pci_set_drvdata(struct pci_dev *pdev, void *data)
{
    pdev->driver_data = data;
}

/** struct rtapi_pcidev - Opaque data structure for the PCI device */
struct rtapi_pcidev;

/** rtapi_pci_get_device - Find a PCI device
 * @vendor: The vendor ID to search for
 * @device: The device ID to search for
 * @from: The device to start searching from. Can be NULL.
 */
extern
struct rtapi_pcidev * rtapi_pci_get_device(__u16 vendor, __u16 device,
					   struct rtapi_pcidev *from);

/** rtapi_pci_put_device - Free a PCI device obtained by rtapi_pci_get_device() */
extern
void rtapi_pci_put_device(struct rtapi_pcidev *dev);

/** rtapi_pci_ioremap - Remap I/O memory
 * Returns NULL on error.
 * @dev: The device
 * @bar: The PCI BAR to remap.
 * @size: The size of the mapping.
 */
extern
void __iomem * rtapi_pci_ioremap(struct rtapi_pcidev *dev, int bar, size_t size);

/** rtapi_pci_iounmap - Unmap an MMIO region
 * @dev: The device
 * @mmio: The MMIO region obtained by rtapi_pci_ioremap()
 */
extern
void rtapi_pci_iounmap(struct rtapi_pcidev *dev, void __iomem *mmio);

static inline
__u8 rtapi_pci_readb(const void __iomem *mmio)
{
#ifdef BUILD_SYS_USER_DSO
	return *((volatile const __u8 __iomem *)mmio);
#else
	return readb(mmio);
#endif
}

static inline
__u16 rtapi_pci_readw(const void __iomem *mmio)
{
#ifdef BUILD_SYS_USER_DSO
	return *((volatile const __u16 __iomem *)mmio);
#else
	return readw(mmio);
#endif
}

static inline
__u32 rtapi_pci_readl(const void __iomem *mmio)
{
#ifdef BUILD_SYS_USER_DSO
	return *((volatile const __u32 __iomem *)mmio);
#else
	return readl(mmio);
#endif
}

static inline
void rtapi_pci_writeb(void __iomem *mmio, unsigned int offset, __u8 value)
{
#ifdef BUILD_SYS_USER_DSO
	*((volatile __u8 __iomem *)mmio) = value;
#else
	writeb(value, mmio);
#endif
}

static inline
void rtapi_pci_writew(void __iomem *mmio, unsigned int offset, __u16 value)
{
#ifdef BUILD_SYS_USER_DSO
	*((volatile __u16 __iomem *)mmio) = value;
#else
	writew(value, mmio);
#endif
}

static inline
void rtapi_pci_writel(void __iomem *mmio, unsigned int offset, __u32 value)
{
#ifdef BUILD_SYS_USER_DSO
	*((volatile __u32 __iomem *)mmio) = value;
#else
	writel(value, mmio);
#endif
}
#endif

#endif // RTAPI_PCI_H