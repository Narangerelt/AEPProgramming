# AEPProgramming
First steps to program on AEP
### Basics and clearing process
1) Check device size:
```
# fdisk -l
```
2) Check filesystem use:
```
# df -Th
```
- **Displaying Bus, NVDIMM, Region, and Namespace Information** 
1) List all active/enabled namespaces:
```
# ndctl list
```
2) To list all active/enabled NVDIMMs, Regions, and Namespaces:
```
# ndctl list -DRN
```
- **Disabling and destroying existing namespaces** 
1) Make sure to disable the namespace before destroying it.
```
# ndctl disable-namespace <namespaceX.Y>
- or -
# ndctl disable-namespace all
```
2) Destroy namespaces:
```
# ndctl destroy-namespace <namespaceX.Y>
- or -
# ndctl destroy-namespace all
- or destroy in force-
# ndctl destroy-namespace -f all
```
- **Unmounting filesystems** 
1) If namespaces cannot be destroyed because of the mounted filesystem, unmount the filesystem first.
```
# umount <file system mount point>
```
2) If filesystems cannot be unmount because they are busy, kill all the processes inside of it.
```
# fuser -k <filesystem mount pount>
```
### Work on Non-Interleaved NVDIMMs, each with their own Region and Namespace
- **Create non-interleaved NVDIMM devices:**
```
# ipmctl create -goal MemoryMode=0 PersistentMemoryType=AppDirectNotInterleaved
# reboot
```
1) Find dimms residing in one socket:
```
# ndctl list -D -U <socket number>
```
2) Create a new NVDIMM namespace in file system DAX mode on the desired region:
```
# ndctl create-namespace --mode=fsdax --region=<region number>
```
See following link to see different options to create namespaces:
https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/8/html/managing_storage_devices/using-nvdimm-persistent-memory-storage_managing-storage-devices#reconfiguring-an-existing-nvdimm-namespace-to-sector-mode_creating-a-sector-namespace-on-an-nvdimm-to-act-as-a-block-device

- **Mount filesystems for each dimms:**
1) Create ext4 or xfs file system for /dev/pmem#:
```
# mkfs.ext4 /dev/pmem#
```
1) Create ext4 or xfs file system for /dev/pmem#:
```
# mkfs.ext4 /dev/pmem#
```
2) Create directory to mount your file system for each your dimm:
```
# mkdir –p /mnt/mem0
```
3) Mount your devices:
```
# mount –t ext4 –o dax /dev/pmem0 /mnt/mem0/
```
3) Check whether your filesystems are correctly mounted:
```
# lsblk
-or
# lsblk -fi
```
- **Create files on each dimms:**
```
# touch /mnt/mem1/file.txt
```
