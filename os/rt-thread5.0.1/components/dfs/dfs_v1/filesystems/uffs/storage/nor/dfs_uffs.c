/*
******************************************************************************
Copyright (C),  Fujian <Company>. Co., Ltd.
File name    ： dfs_uffs.c
Description  ： uffs文件系统与dfs的接口
Compile      ： 
Author       ： emb_wlq
Version      ： v1.0
Date         ： 
Others       ： 
History      ： 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
1) 日期：2014.02.20         修改者：emb_wlq
   内容：v1.0

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
2）...

******************************************************************************
*/
/* 头文件 */
#include <rtthread.h>

#include "w25qxx.h"

#include <dfs_fs.h>
#include <dfs_file.h>
#include "dfs_uffs.h"

#include "uffs/uffs_fd.h" /* posix file api is here */
#include "uffs/uffs_mtb.h"
#include "uffs/uffs_mem.h"
#include "uffs/uffs_utils.h"

/*
******************************************************************************
                              局部宏定义
******************************************************************************
*/
/*
 * RT-Thread DFS Interface for uffs
 */
#define UFFS_DEVICE_MAX         1    /* the max partions on a nand deivce*/
#define UFFS_MOUNT_PATH_MAX     128  /* the mount point max length */
#define FILE_PATH_MAX           256  /* the longest file path */

/*
******************************************************************************
                           结构体或联合体定义
******************************************************************************
*/
struct _flash_dev
{
    st_flash_device_t * dev;
    struct uffs_StorageAttrSt storage;
    uffs_Device uffs_dev;
    uffs_MountTable mount_table;
    char mount_path[UFFS_MOUNT_PATH_MAX];
    void * data;  /* when uffs use static buf, it will save ptr here */ 
};

/*
******************************************************************************
                              外部变量定义
******************************************************************************
*/

/*
******************************************************************************
                              全局变量定义
******************************************************************************
*/
/* make sure the following struct var had been initilased to 0! */
static struct _flash_dev flash_part[UFFS_DEVICE_MAX];

/* static alloc the memory */
#if CONFIG_USE_STATIC_MEMORY_ALLOCATOR > 0
#define PAGE_SPARE_SIZE   			16
#define PAGE_DATA_SIZE    			(W25QXX_PAGE_SIZE - PAGE_SPARE_SIZE)
#define PAGES_PER_BLOCK   			W25QXX_PAGE_PER_SECTOR
#define TOTAL_BLOCKS      			W25QXX_SECTOR_NUM

#define PAGE_SIZE					(PAGE_DATA_SIZE + PAGE_SPARE_SIZE)
#define BLOCK_DATA_SIZE				(PAGES_PER_BLOCK * PAGE_DATA_SIZE)
#define TOTAL_DATA_SIZE				(TOTAL_BLOCKS * BLOCK_DATA_SIZE)
#define BLOCK_SIZE					(PAGES_PER_BLOCK * PAGE_SIZE)
#define TOTAL_SIZE					(BLOCK_SIZE * TOTAL_BLOCKS)
static int static_buffer_pool[UFFS_DEVICE_MAX][UFFS_STATIC_BUFF_SIZE(PAGES_PER_BLOCK, PAGE_SIZE, TOTAL_BLOCKS) / sizeof(int)] SECTION(".fsmem");
#endif

/*
******************************************************************************
                              局部函数声明
******************************************************************************
*/

/*
******************************************************************************
                              局部表定义
******************************************************************************
*/

/*
=====================================================================
                              局部函数
=====================================================================
*/
/*
*************************************************************************
Function    : 函数名称
Description : 函数功能、性能描述
Input       : 输入参数说明，包括参数的作用、取值说明及参数间关系。
Output      : 输出参数说明。
Return      : 函数返回值说明
Others      : 其它说明
************************************************************************
*/
/*
*************************************************************************
Function      : uffs_result_to_dfs
Description   : uffs错误转换为dfs错误
Input         : 
Output        : 
Return        : 
Others        : 
************************************************************************
*/
static int uffs_result_to_dfs(int result)
{
    int status = -1;

    result = result < 0 ? -result : result;
    switch (result)
    {
    case UENOERR:/** no error */
        break;
    case UEACCES:/** Tried to open read-only file for writing, or files sharing mode
                   does not allow specified operations, or given path is directory */
        status = -EINVAL;
        break;/* no suitable */
    case UEEXIST:   /** _O_CREAT and _O_EXCL flags specified, but filename already exists */
        status = -EEXIST;
        break;
    case UEINVAL:  /** Invalid oflag or pmode argument */
        status = -EINVAL;
        break;
    case UEMFILE: /** No more file handles available(too many open files)  */
        status = -1;
        break;
    case UENOENT: /** file or path not found */
        status = -ENOENT;
        break;
    case UETIME: /** can't set file time */
        status = -1;
        break;
    case UEBADF: /** invalid file handle */
        status = -EBADF;
        break;
    case UENOMEM:/** no enough memory */
        status = -ENOSPC;
        break;
    case UEIOERR: /** I/O error from lower level flash operation */
        status = -EIO;
        break;
    case UENOTDIR: /** Not a directory */
        status = -ENOTDIR;
        break;
    case UEISDIR: /** Is a directory */
        status = -EISDIR;
        break;
    case UEUNKNOWN_ERR:
    default:
        status = -1;
        break; /* unknown error! */
    }

    return status;
}
/*
*************************************************************************
Function      : _device_init
Description   : 设备初始化
Input         : 
Output        : 
Return        : 
Others        : 
************************************************************************
*/
static URET _device_init(uffs_Device *dev)
{
    dev->attr->_private = NULL; // hook nand_chip data structure to attr->_private
    dev->ops = (struct uffs_FlashOpsSt *)&uffs_Ops;

    return U_SUCC;
}

/*
*************************************************************************
Function      : _device_release
Description   : 设备释放
Input         : 
Output        : 
Return        : 
Others        : 
************************************************************************
*/
static URET _device_release(uffs_Device *dev)
{
    return U_SUCC;
}

/*
*************************************************************************
Function      : init_uffs_fs
Description   : uffs文件系统初始化
Input         : 
Output        : 
Return        : 
Others        : 
************************************************************************
*/
static int init_uffs_fs(struct _flash_dev * flash_part)
{
    uffs_MountTable * mtb;
    st_flash_device_t *flash;
    struct uffs_StorageAttrSt * flash_storage;

    mtb = &flash_part->mount_table;
    flash = flash_part->dev;
    flash_storage = &flash_part->storage;

    /* setup nand storage attributes */
    uffsSetupStorage(flash_storage, flash);

    /* register mount table */
    if(mtb->dev)
    {
        /* set memory allocator for uffs */
#if CONFIG_USE_SYSTEM_MEMORY_ALLOCATOR > 0
        uffs_MemSetupSystemAllocator(&mtb->dev->mem);
#endif
#if CONFIG_USE_STATIC_MEMORY_ALLOCATOR > 0
		uffs_MemSetupStaticAllocator(&mtb->dev->mem, flash_part->data, UFFS_STATIC_BUFF_SIZE(PAGES_PER_BLOCK, PAGE_SIZE, TOTAL_BLOCKS));
#endif

        /* setup device init/release entry */
        mtb->dev->Init = _device_init;
        mtb->dev->Release = _device_release;
        mtb->dev->attr = flash_storage;

        uffs_RegisterMountTable(mtb);
    }
    /* mount uffs partion on nand device */
    return uffs_Mount(flash_part->mount_path) == U_SUCC ? 0 : -1;
}

/*
*************************************************************************
Function      : dfs_uffs_mount
Description   : 文件系统挂载
Input         : 
Output        : 
Return        : 
Others        : 
************************************************************************
*/
static int dfs_uffs_mount(struct dfs_filesystem* fs, unsigned long rwflag, const void* data)
{
    rt_base_t index;
    uffs_MountTable * mount_part;
    st_flash_device_t *dev;
    
    RT_ASSERT(rt_strlen(fs->path) < (UFFS_MOUNT_PATH_MAX-1));
    dev = (pst_flash_device_t)(fs->dev_id);

    /*1. find a empty entry in partition table */
    for (index = 0; index < UFFS_DEVICE_MAX ; index ++)
    {
        if (flash_part[index].dev == RT_NULL)
            break;
    }
    if (index == UFFS_DEVICE_MAX)return -ENOENT;

    /*2. fill partition structure */
    flash_part[index].dev = dev;

    /* make a right mount path for uffs, end with '/' */
    rt_snprintf(flash_part[index].mount_path, UFFS_MOUNT_PATH_MAX, "%s/", fs->path);
    if (flash_part[index].mount_path[1] == '/')flash_part[index].mount_path[1] = 0;

    mount_part = &(flash_part[index].mount_table);
    mount_part->mount   = flash_part[index].mount_path;
    mount_part->dev = &(flash_part[index].uffs_dev);
    rt_memset(mount_part->dev, 0, sizeof(uffs_Device));//in order to make uffs happy.
    mount_part->dev->_private = dev;   /* save dev_id into uffs */
    mount_part->start_block = dev->sector_start;
    mount_part->end_block = dev->sector_end;
    /*3. mount uffs */
    if (init_uffs_fs(&flash_part[index]) < 0)
    {
        return uffs_result_to_dfs(uffs_get_error());
    }
    return 0;
}

/*
*************************************************************************
Function      : dfs_uffs_unmount
Description   : 文件系统卸载
Input         : 
Output        : 
Return        : 
Others        : 
************************************************************************
*/
static int dfs_uffs_unmount(struct dfs_filesystem* fs)
{
    rt_base_t index;
    int result;

    /* find the device index and then unmount it */
    for (index = 0; index < UFFS_DEVICE_MAX; index++)
    {
        if (flash_part[index].dev == (pst_flash_device_t)(fs->dev_id))
        {
            flash_part[index].dev = RT_NULL;
            result = uffs_UnMount(flash_part[index].mount_path);
            if (result != U_SUCC)
                break;

            result = uffs_UnRegisterMountTable(&flash_part[index].mount_table);
            return (result == U_SUCC) ? RT_EOK : -1;
        }
    }
    return -ENOENT;
}

/*
*************************************************************************
Function      : dfs_uffs_mkfs
Description   : 
Input         : 
Output        : 
Return        : 
Others        : 
************************************************************************
*/
static int dfs_uffs_mkfs(rt_device_t dev_id)
{
    rt_base_t index;

    /*1. find the device index */
    for (index = 0; index < UFFS_DEVICE_MAX; index++)
    {
        if (flash_part[index].dev == (st_flash_device_t *)dev_id)
            break;
    }

    if (index == UFFS_DEVICE_MAX)
    {
        /* can't find device driver */
        return -ENOENT;
    }

    return (uffs_format(flash_part[index].mount_path));

#if 0
    rt_uint32_t block;
    st_flash_device_t * mtd;
    
    /*2. then unmount the partition */
    uffs_UnMount(flash_part[index].mount_path);
    mtd = flash_part[index].dev;

    /*3. erase all blocks on the partition */
    block = mtd->block_start;
    for (; block <= mtd->block_end; block++)
    {
        rt_device_control((rt_device_t)mtd, FLASH_CMD_ERASE_BLOCK, (void *)&block);
        if (rt_device_control((rt_device_t)mtd, FLASH_CMD_CHECK_BLOCK, (void *)&block) != RT_EOK)
        {
            rt_kprintf("found bad block %d\n", block);

            /* 登记坏块 */
            
        }
        else
            rt_kprintf("erase block:%d\n", block);
    }

    /*4. remount it */
    if (init_uffs_fs(&flash_part[index]) < 0)
    {
        return uffs_result_to_dfs(uffs_get_error());
    }
    return RT_EOK;
#endif
}

/*
*************************************************************************
Function      : dfs_uffs_statfs
Description   : 
Input         : 
Output        : 
Return        : 
Others        : 
************************************************************************
*/
static int dfs_uffs_statfs(struct dfs_filesystem* fs, struct statfs *buf)
{
    rt_base_t index;
    st_flash_device_t * mtd = (pst_flash_device_t)(fs->dev_id);

    RT_ASSERT(mtd != RT_NULL);

    /* find the device index */
    for (index = 0; index < UFFS_DEVICE_MAX; index++)
    {
        if (flash_part[index].dev == (void *)mtd)
            break;
    }
    if (index == UFFS_DEVICE_MAX)
        return -ENOENT;
    
    buf->f_bsize = mtd->page_size * mtd->pages_per_sector;
    buf->f_blocks = (mtd->sector_end - mtd->sector_start + 1);
    buf->f_bfree = uffs_GetDeviceFree(&flash_part[index].uffs_dev) / buf->f_bsize;
    
    return 0;
}

/*
*************************************************************************
Function      : dfs_uffs_open
Description   : 
Input         : 
Output        : 
Return        : 
Others        : 
************************************************************************
*/
static int dfs_uffs_open(struct dfs_fd* file)
{
    int fd;
    int oflag, mode;
    char * file_path;

    oflag = file->flags;
    if (oflag & O_DIRECTORY)   /* operations about dir */
    {
        uffs_DIR * dir;

        if (oflag & O_CREAT)   /* create a dir*/
        {
            if (uffs_mkdir(file->path) < 0)
                return uffs_result_to_dfs(uffs_get_error());
        }
        /* open dir */
        file_path = rt_malloc(FILE_PATH_MAX);
        if(file_path == RT_NULL)
            return -ENOMEM;         

        if (file->path[0] == '/' && !(file->path[1] == 0))
            rt_snprintf(file_path, FILE_PATH_MAX, "%s/", file->path);
        else
        {
            file_path[0] = '/';
            file_path[1] = 0;
        }

        dir = uffs_opendir(file_path);

        if (dir == RT_NULL)
        {
            rt_free(file_path);         
            return uffs_result_to_dfs(uffs_get_error());
        }
        /* save this pointer,will used by  dfs_uffs_getdents*/
        file->data = dir;
        rt_free(file_path);
        return RT_EOK;
    }
    /* regular file operations */
    /* int uffs_open(const char *name, int oflag, ...); what is this?
     * uffs_open can open dir!!  **/
    mode = 0;
    if (oflag & O_RDONLY) mode |= UO_RDONLY;
    if (oflag & O_WRONLY) mode |= UO_WRONLY;
    if (oflag & O_RDWR)   mode |= UO_RDWR;
    /* Opens the file, if it is existing. If not, a new file is created. */
    if (oflag & O_CREAT) mode |= UO_CREATE;
    /* Creates a new file. If the file is existing, it is truncated and overwritten. */
    if (oflag & O_TRUNC) mode |= UO_TRUNC;
    /* Creates a new file. The function fails if the file is already existing. */
    if (oflag & O_EXCL) mode |= UO_EXCL;

    fd = uffs_open(file->path, mode);
    if (fd < 0)
    {
        return uffs_result_to_dfs(uffs_get_error());
    }

    /* save this pointer, it will be used when calling read()，write(),
     * flush(), seek(), and will be free when calling close()*/

    file->data = (void *)fd;
    file->pos  = uffs_seek(fd, 0, USEEK_CUR);
    file->size = uffs_seek(fd, 0, USEEK_END);
    uffs_seek(fd, file->pos, USEEK_SET);

    if (oflag & O_APPEND)
    {
        file->pos = uffs_seek(fd, 0, USEEK_END);
    }
    return 0;
}

/*
*************************************************************************
Function      : dfs_uffs_close
Description   : 
Input         : 
Output        : 
Return        : 
Others        : 
************************************************************************
*/
static int dfs_uffs_close(struct dfs_fd* file)
{
    int oflag;
    int fd;

    oflag = file->flags;
    if (oflag & O_DIRECTORY)
    {
        /* operations about dir */
        if (uffs_closedir((uffs_DIR *)(file->data)) < 0)
            return uffs_result_to_dfs(uffs_get_error());

        return 0;
    }
    /* regular file operations */
    fd = (int)(file->data);

    if (uffs_close(fd) == 0)
        return 0;

    return uffs_result_to_dfs(uffs_get_error());
}

/*
*************************************************************************
Function      : dfs_uffs_ioctl
Description   : 
Input         : 
Output        : 
Return        : 
Others        : 
************************************************************************
*/
static int dfs_uffs_ioctl(struct dfs_fd * file, int cmd, void* args)
{
    return -ENOSYS;
}

/*
*************************************************************************
Function      : 
Description   : 
Input         : 
Output        : 
Return        : 
Others        : 
************************************************************************
*/
static int dfs_uffs_read(struct dfs_fd * file, void* buf, size_t len)
{
    int fd;
    int char_read;

    fd = (int)(file->data);
    char_read = uffs_read(fd, buf, len);
    if (char_read < 0)
        return uffs_result_to_dfs(uffs_get_error());

    /* update position */
    file->pos = uffs_seek(fd, 0, USEEK_CUR);
    return char_read;
}

/*
*************************************************************************
Function      : dfs_uffs_write
Description   : 
Input         : 
Output        : 
Return        : 
Others        : 
************************************************************************
*/
static int dfs_uffs_write(struct dfs_fd* file, const void* buf, size_t len)
{
    int fd;
    int char_write;

    fd = (int)(file->data);

    char_write = uffs_write(fd, buf, len);
    if (char_write < 0)
        return uffs_result_to_dfs(uffs_get_error());

    /* update position */
    file->pos = uffs_seek(fd, 0, USEEK_CUR);
    return char_write;
}

/*
*************************************************************************
Function      : dfs_uffs_flush
Description   : 
Input         : 
Output        : 
Return        : 
Others        : 
************************************************************************
*/
static int dfs_uffs_flush(struct dfs_fd* file)
{
    int fd;
    int result;

    fd = (int)(file->data);

    result = uffs_flush(fd);
    if (result < 0 )
        return uffs_result_to_dfs(uffs_get_error());
    return 0;
}

/*
*************************************************************************
Function      : uffs_seekdir
Description   : 
Input         : 
Output        : 
Return        : 
Others        : 
************************************************************************
*/
int uffs_seekdir(uffs_DIR *dir, long offset)
{
    int i = 0;

    while(i < offset)
    {   
        if (uffs_readdir(dir) == RT_NULL)
            return -1;
        i++;
    } 
    return 0;
}

/*
*************************************************************************
Function      : dfs_uffs_seek
Description   : 
Input         : 
Output        : 
Return        : 
Others        : 
************************************************************************
*/
static int dfs_uffs_seek(struct dfs_fd* file, rt_off_t offset)
{
    int result;

    /* set offset as current offset */
    if (file->type == FT_DIRECTORY)
    {
        uffs_rewinddir((uffs_DIR *)(file->data));
        result = uffs_seekdir((uffs_DIR *)(file->data), offset/sizeof(struct dirent));
        if (result >= 0)
        {
            file->pos = offset; 
            return offset;
        }
    }
    else if (file->type == FT_REGULAR)
    {
        result = uffs_seek((int)(file->data), offset, USEEK_SET);
        if (result >= 0)    
            return offset;
    }

    return uffs_result_to_dfs(uffs_get_error());
}

/*
*************************************************************************
Function      : dfs_uffs_getdents
Description   : 
Input         : 
Output        : 
Return        : 
Others        : 
************************************************************************
*/
static int dfs_uffs_getdents(struct dfs_fd* file, struct dirent* dirp, uint32_t count)
{
    rt_uint32_t index;
    char * file_path;
    struct dirent* d;
    uffs_DIR* dir;
    struct uffs_dirent * uffs_d;
    
    dir = (uffs_DIR*)(file->data);
    RT_ASSERT(dir != RT_NULL);
    
    /* round count, count is always 1 */
    count = (count / sizeof(struct dirent)) * sizeof(struct dirent);
    if (count == 0) return -EINVAL;

    /* allocate file name */
    file_path = rt_malloc(FILE_PATH_MAX);
    if (file_path == RT_NULL)
        return -ENOMEM;
        
    index = 0;
    /* usually, the while loop should only be looped only once! */
    while (1)
    {
        struct uffs_stat s;
        
        d = dirp + index;

        uffs_d = uffs_readdir(dir);
        if (uffs_d == RT_NULL)
        {
            rt_free(file_path);
            return (uffs_result_to_dfs(uffs_get_error()));
        }

        if (file->path[0] == '/' && !(file->path[1] == 0))
            rt_snprintf(file_path, FILE_PATH_MAX, "%s/%s", file->path, uffs_d->d_name);
        else
            rt_strncpy(file_path, uffs_d->d_name, FILE_PATH_MAX);

        uffs_stat(file_path, &s); 
        switch(s.st_mode & US_IFMT)   /* file type mark */
        {
        case US_IFREG: /* directory */
            d->d_type = DT_REG;
            break;
        case US_IFDIR: /* regular file */
            d->d_type = DT_DIR;
            break;
        case US_IFLNK: /* symbolic link */
        case US_IREAD: /* read permission */
        case US_IWRITE:/* write permission */
        default:
            d->d_type = DT_UNKNOWN;
            break;
        }

        /* write the rest args of struct dirent* dirp  */
        d->d_namlen = rt_strlen(uffs_d->d_name);
        d->d_reclen = (rt_uint16_t)sizeof(struct dirent);
        rt_strncpy(d->d_name, uffs_d->d_name, rt_strlen(uffs_d->d_name) + 1);

        index ++;
        if (index * sizeof(struct dirent) >= count)
            break;
    }
    
    /* free file name buf */
    rt_free(file_path);
    
    if (index == 0)
        return uffs_result_to_dfs(uffs_get_error());

    file->pos += index * sizeof(struct dirent);

    return index * sizeof(struct dirent);
}

/*
*************************************************************************
Function      : dfs_uffs_unlink
Description   : 
Input         : 
Output        : 
Return        : 
Others        : 
************************************************************************
*/
static int dfs_uffs_unlink(struct dfs_filesystem* fs, const char* path)
{
    int result;
    struct uffs_stat s;

    /* judge file type, dir is to be delete by uffs_rmdir, others by uffs_remove */
    if (uffs_lstat(path, &s) < 0)
    {
        return uffs_result_to_dfs(uffs_get_error());
    }

    switch(s.st_mode & US_IFMT)
    {
    case US_IFREG:
        result = uffs_remove(path);
        break;
    case US_IFDIR:
        result = uffs_rmdir(path);
        break;
    default:
        /* unknown file type */
        return -1;
    }
    if (result < 0)
        return uffs_result_to_dfs(uffs_get_error());

    return 0;
}

/*
*************************************************************************
Function      : dfs_uffs_rename
Description   : 
Input         : 
Output        : 
Return        : 
Others        : 
************************************************************************
*/
static int dfs_uffs_rename(struct dfs_filesystem* fs, const char* oldpath, const char* newpath)
{
    int result;
    
    result = uffs_rename(oldpath, newpath);
    if (result < 0)
        return uffs_result_to_dfs(uffs_get_error());

    return 0;
}

/*
*************************************************************************
Function      : dfs_uffs_stat
Description   : 
Input         : 
Output        : 
Return        : 
Others        : 
************************************************************************
*/
static int dfs_uffs_stat(struct dfs_filesystem* fs, const char *path, struct stat *st)
{
    int result;
    struct uffs_stat s;

    result = uffs_stat(path, &s);
    if (result < 0)
        return uffs_result_to_dfs(uffs_get_error());

    /* convert uffs stat to dfs stat structure */
    /* FIXME, these field may not be the same */
    st->st_dev  = 0;
    st->st_mode = s.st_mode;
    st->st_size = s.st_size;
    st->st_mtime = s.st_mtime;

    return 0;
}

/*
*************************************************************************
Function      : dfs_uffs_ops
Description   : 接口
Input         : 
Output        : 
Return        : 
Others        : 
************************************************************************
*/
static const struct dfs_file_ops dfs_uffs_fops = 
{
    dfs_uffs_open,
    dfs_uffs_close,
    dfs_uffs_ioctl,
    dfs_uffs_read,
    dfs_uffs_write,
    dfs_uffs_flush,
    dfs_uffs_seek,
    dfs_uffs_getdents,
};

static const struct dfs_filesystem_ops dfs_uffs_ops =
{
    "uffs", /* file system type: uffs */
#if RTTHREAD_VERSION >= 10100
    DFS_FS_FLAG_FULLPATH,
#else
#error "uffs can only work with rtthread whose version should >= 1.01\n"
#endif
    &dfs_uffs_fops,

    dfs_uffs_mount,
    dfs_uffs_unmount,
    dfs_uffs_mkfs,
    dfs_uffs_statfs,

    dfs_uffs_unlink,
    dfs_uffs_stat,
    dfs_uffs_rename,
};

/*
=====================================================================
                              全局函数
=====================================================================
*/
/*
*************************************************************************
Function    : 函数名称
Description : 函数功能、性能描述
Input       : 输入参数说明，包括参数的作用、取值说明及参数间关系。
Output      : 输出参数说明。
Return      : 函数返回值说明
Others      : 其它说明
************************************************************************
*/
/*
*************************************************************************
Function      : dfs_uffs_init
Description   : 初始化
Input         : 
Output        : 
Return        : 
Others        : 
************************************************************************
*/
int dfs_uffs_init(void)
{
    rt_uint32_t i;

    for (i = 0; i < UFFS_DEVICE_MAX; i++)
    {
        rt_memset(&flash_part[i], 0, sizeof(struct _flash_dev));
#if CONFIG_USE_STATIC_MEMORY_ALLOCATOR > 0
		flash_part[i].data = &static_buffer_pool[i][0];
#endif
    }

    /* register uffs file system */
    dfs_register(&dfs_uffs_ops);

    if (uffs_InitObjectBuf() == U_SUCC)
    {
        if (uffs_DirEntryBufInit() == U_SUCC)
        {
            uffs_InitGlobalFsLock();
            return RT_EOK;
        }
    }
 
    return -RT_ERROR;
}
INIT_COMPONENT_EXPORT(dfs_uffs_init);

