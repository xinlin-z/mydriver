#define pr_fmt(fmt) "%s:%s:%d: " fmt, KBUILD_MODNAME, __func__, __LINE__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include "mychar.h"

MODULE_LICENSE("GPL");

#define MINOR_FIRST   3
#define MINOR_NUM     4
#define DEFAULT_LEN   (1024*4)

static unsigned int major = 0;
static unsigned int minor_pos = 0; /* count of successful cdev_add */

static unsigned long init_content_len = DEFAULT_LEN;
module_param(init_content_len, ulong, S_IRUGO);

struct file_operations mychar_fop;
struct append_mem {
    struct append_mem *next;
    unsigned long curlen;
    char *mm;
};
struct mychar_dev {
    char *cont;
    size_t len;
    unsigned long content_len;
    struct semaphore sema;
    struct cdev mycdev;
    struct append_mem *am;
} *pmychar = NULL;


int mychar_open(struct inode *inode, struct file *fp) {
    unsigned int flags;

    /* check flags */
    flags = fp->f_flags & O_ACCMODE;
    if ((flags != O_RDONLY) && (flags != O_WRONLY))
        return -EPERM;  // operation not permitted

    fp->private_data = (void *)container_of(inode->i_cdev,
                                            struct mychar_dev, mycdev);
    return 0;
}


int mychar_release(struct inode *inode, struct file *fp) {
    return 0;
}


ssize_t mychar_read(struct file *fp, char __user *buf,
                    size_t count, loff_t *f_pos) {
    unsigned long left;
    struct mychar_dev *dev=(struct mychar_dev *)fp->private_data;
    
    if (down_interruptible(&dev->sema))
        return -ERESTARTSYS;

    pr_info("read called, count %zu.\n", count);

    if (*f_pos >= dev->len) {
        pr_info("read call end, return count 0.\n");
        up(&dev->sema);
        return 0;
    }
    
    /* update count if reach end */
    if ((*f_pos+count) > dev->len)
        count = dev->len - *f_pos;

    /* must use copy_to_user, can't derefer user-space */
    if ((left = copy_to_user(buf,dev->cont+*f_pos,count))) {
        if (left != count) {
            count -= left;
            pr_err("copy_to_user return less than count!\n");
        }
        else {
            up(&dev->sema);
            return -EFAULT;
        }
    }

    /* update *f_pos */
    *f_pos += count;

    /* return how many char readed */
    pr_info("read call end, return count %zu.\n", count);
    up(&dev->sema);
    return count;
}


ssize_t mychar_write(struct file *fp, const char __user *buf,
                     size_t count, loff_t *f_pos) {
    unsigned long left;
    struct mychar_dev *dev=(struct mychar_dev *)fp->private_data;

    if (down_interruptible(&dev->sema))
        return -ERESTARTSYS;

    pr_info("write called, count %zu, *f_pos=%lld.\n", count, *f_pos);

    if (*f_pos >= dev->content_len) {
        pr_info("write call end, return count 0.\n");
        up(&dev->sema);
        return -ENOSPC;
    }

    if ((*f_pos+count) > dev->content_len)
        count = dev->content_len - *f_pos;

    if ((left = copy_from_user(dev->cont+*f_pos,buf,count))) {
        if (left != count) {
            count -= left;
            pr_err("copy_from_user return less than count!\n");
        }
        else {
            up(&dev->sema);
            return -EFAULT;
        }
    }

    *f_pos += count;
    if (dev->len < *f_pos)
        dev->len = *f_pos;  // update real length
    pr_info("write call end, return count %zu.\n", count);
    up(&dev->sema);
    return count;
}


loff_t mychar_llseek(struct file *fp, loff_t offs, int whence) {
    struct mychar_dev *dev=(struct mychar_dev *)fp->private_data;
    loff_t pos;

    if (down_interruptible(&dev->sema))
        return -ERESTARTSYS;

    pr_info("seek called, offset = %lld, whence = %d\n", offs, whence);

    switch (whence) {
        case 0:  // SEEK_SET
            pos = offs;
            break;
        case 1:  // SEEK_CUR
            pos = fp->f_pos + offs;
            break;
        case 2:  // SEEK_END
            pos = dev->len + offs;
            break;
        default:
            return -EINVAL;
    }

    if (pos < 0) return -EINVAL;
    /* modify fp->f_pos directly here */
    fp->f_pos = pos;
    pr_info("new position: %lld\n", pos);
    up(&dev->sema);
    return pos;
}


long mychar_ioctl(struct file *fp, unsigned int cmd, unsigned long arg) {
    struct mychar_dev *dev=(struct mychar_dev *)fp->private_data;
    char *tmp;
    struct ioc_read iocr;
    long rtv = 0;
    unsigned long tmpul;

    if (down_interruptible(&dev->sema))
        return -ERESTARTSYS;

    pr_info("ioctl, cmd = 0x%X\n", cmd);

    /* check type(magic) and cmd number */
    if ((_IOC_TYPE(cmd) != MYCHAR_IOC_MAGIC) 
            || (_IOC_NR(cmd) > MYCHAR_IOC_MAXNR)) {
        rtv = -ENOTTY;
        goto end;
    }

    /* check user space */
    if (!access_ok((void __user *)arg, _IOC_SIZE(cmd))) {
        up(&dev->sema);
        rtv = -EFAULT;
        goto end;
    }

    pr_info("access_ok passed, switch ioctl command\n");

    switch (cmd) {
        case MYCHAR_IOC_RESET:
            if ((tmp = kmalloc(DEFAULT_LEN,GFP_KERNEL)) == NULL) {
                rtv = -ENOMEM;
                goto end;
            }
            if (dev->cont != NULL)
                kfree(dev->cont);
            dev->cont = tmp;
            memset(dev->cont, 0, DEFAULT_LEN);
            dev->content_len = DEFAULT_LEN;
            dev->len = 0;
            break;

        case MYCHAR_IOC_QUERY:
            rtv = __put_user(dev->content_len, (unsigned long __user *)arg);
            goto end;

        case MYCHAR_IOC_QUERY2:
            rtv = dev->content_len;
            goto end;

        case MYCHAR_IOC_SET:
            if ((rtv = __get_user(tmpul, (unsigned long __user *)arg)))
                goto end;
            if ((tmp=kmalloc(tmpul,GFP_KERNEL)) == NULL) {
                rtv = -ENOMEM;
                goto end;
            }
            if (dev->cont != NULL)
                kfree(dev->cont);
            dev->cont = tmp;
            memset(dev->cont, 0, tmpul);
            dev->content_len = tmpul;
            dev->len = 0;
            break;

        case MYCHAR_IOC_QNS:
            if ((rtv = __get_user(tmpul, (unsigned long __user *)arg)))
                goto end;
            if ((tmp=kmalloc(tmpul,GFP_KERNEL)) == NULL) {
                rtv = -ENOMEM;
                goto end;
            }
            if (dev->cont != NULL)
                kfree(dev->cont);
            dev->cont = tmp;
            memset(dev->cont, 0, tmpul);
            dev->len = 0;
            if ((rtv = __put_user(dev->content_len,
                                  (unsigned long __user *)arg))) {
                dev->content_len = tmpul;
                goto end;
            }
            dev->content_len = tmpul;
            break;

        case MYCHAR_IOC_CLS:
            if (dev->cont != NULL) {
                kfree(dev->cont);
                dev->cont = NULL;
            }
            dev->content_len = 0;
            dev->len = 0;
            break;

        case MYCHAR_IOC_CNS:
            if ((rtv = __get_user(tmpul, (unsigned long __user *)arg)))
                goto end;
            if (dev->cont != NULL) {
                kfree(dev->cont);
                dev->content_len = 0;
                dev->len = 0;
            }
            if ((tmp=kmalloc(tmpul,GFP_KERNEL)) == NULL) {
                dev->content_len = 0;
                rtv = -ENOMEM;
                goto end;
            }
            dev->cont = tmp;
            memset(dev->cont, 0, tmpul);
            dev->content_len = tmpul;
            break;

        case MYCHAR_IOC_READ:
            if (!capable(CAP_SYS_ADMIN)) {
                rtv = -EPERM;
                goto end;
            }
            if ((rtv = __copy_from_user(&iocr, (const void *)arg,
                                        sizeof(struct ioc_read)))) {
                rtv = -EFAULT;
                goto end;
            }
            if (dev->content_len > iocr.skip) {
                tmpul = dev->content_len - iocr.skip;
                if (tmpul >= MYCHAR_IOC_READ_LEN)
                    tmpul = MYCHAR_IOC_READ_LEN;
                if ((rtv = __copy_to_user(((struct ioc_read *)arg)->content,
                                          (const void *)&dev->cont[iocr.skip],
                                          tmpul))) {
                    if (rtv > 0)
                        rtv = tmpul - rtv;
                    goto end;
                }
            }
            rtv = tmpul;
            break;   // return how many bytes copied

        case MYCHAR_IOC_AM:
            if (!capable(CAP_SYS_ADMIN)) {
                rtv = -EPERM;
                goto end;
            }
            if ((rtv = __get_user(tmpul, (unsigned long __user *)arg)))
                goto end;
            pr_info("__get_user, tmpul = %lu\n", tmpul);
            do {
                unsigned long count = 0;
                struct append_mem *next, *prev;
                char *bmm;
                struct append_mem *smm = (struct append_mem*)kmalloc(
                                     sizeof(struct append_mem),GFP_KERNEL);
                if (smm == NULL) {
                    rtv = -ENOMEM;
                    goto end;
                }
                memset(smm, 0, sizeof(struct append_mem));
                next = dev->am;
                while (next != NULL) {
                    count += next->curlen;
                    prev = next;
                    next = next->next;
                }
                if (dev->am == NULL)
                    dev->am = next = smm;  // set head
                else prev->next = next = smm;
                bmm = kmalloc(tmpul, GFP_KERNEL);
                if (bmm == NULL) {
                    kfree(next);
                    if (dev->am == next)
                        dev->am = next = NULL;
                    else prev->next = NULL;
                    rtv = -ENOMEM;
                    goto end;
                }
                // no need to clean this memory block
                next->curlen = tmpul;
                next->mm = bmm;
                rtv = count + tmpul;
                pr_info("at last, rtv = %lu\n", rtv);
            } while(0);
            break;

        case MYCHAR_IOC_FALL:
            if (!capable(CAP_SYS_ADMIN)) {
                rtv = -EPERM;
                goto end;
            }
            do {
                struct append_mem *prev;
                struct append_mem *next = dev->am;
                dev->am = NULL;
                while (next != NULL) {
                    prev = next;
                    next = next->next;
                    kfree(prev->mm);
                    kfree(prev);
                }
            } while(0);
            break;

        default: // should not be here
            rtv = -ENOTTY;
    }

end:
    up(&dev->sema);
    return rtv;
}


struct file_operations mychar_fop = {
    .owner   = THIS_MODULE,
    .open    = mychar_open,
    .release = mychar_release,
    .read    = mychar_read,
    .write   = mychar_write,
    .llseek  = mychar_llseek,
    .unlocked_ioctl = mychar_ioctl,
};


static void mychar_exit(void) {
    int i;

    if (pmychar != NULL) {
        for (i=0; i<MINOR_NUM; ++i)
            if (pmychar[i].cont != NULL) kfree(pmychar[i].cont);
        for (i=0; i<minor_pos; ++i)
            cdev_del(&pmychar[i].mycdev);
        kfree(pmychar);
    }
    unregister_chrdev_region(MKDEV(major,MINOR_FIRST), MINOR_NUM);
    pr_notice("exit.\n");
}


static int __init mychar_init(void) {
    int rn, i;
    dev_t dev;

    /* get a major number */
    if ((rn = alloc_chrdev_region(&dev,MINOR_FIRST,MINOR_NUM,"mychar"))) {
        pr_warn("can't get major number, err %d.\n", rn);
        return rn;
    }
    major = MAJOR(dev);
    pr_info("major is %d, %d minor start from %d.\n",
                major, MINOR_NUM, MINOR_FIRST);

    /* alloc mychar, init mutex */
    pmychar = kmalloc(MINOR_NUM*sizeof(struct mychar_dev), GFP_KERNEL);
    if (pmychar == NULL) {
        mychar_exit();
        return -ENOMEM;
    }
    memset(pmychar, 0, MINOR_NUM*sizeof(struct mychar_dev));
    for (i=0; i<MINOR_NUM; ++i) {
        if ((pmychar[i].cont=kmalloc(init_content_len,GFP_KERNEL)) == NULL) {
            mychar_exit();
            return -ENOMEM;
        }
        sema_init(&pmychar[i].sema, 1); // 1 is for mutex
        pmychar[i].content_len = init_content_len;
        memset(pmychar[i].cont, 0, init_content_len);
    }
    pr_info("alloc %lu bytes memory for all successfully.\n",init_content_len);

    /* init and add cdev */
    for (i=0; i<MINOR_NUM; ++i) {
        cdev_init(&pmychar[i].mycdev, &mychar_fop);
        if((rn = cdev_add(&pmychar[i].mycdev,MKDEV(major,MINOR_FIRST+i),1))){
            pr_err("cdev_add err %d, minor %d.\n", rn, i+MINOR_FIRST);
            mychar_exit();
            return rn;
        }
        ++minor_pos;
    }
    pr_notice("%d devices added successfully.\n", minor_pos);

    /* success return */
    return 0;
}


module_init(mychar_init);
module_exit(mychar_exit);
