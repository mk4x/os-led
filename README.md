# Kernel Development Command Reference

Complete reference for all commands used in Raspberry Pi kernel development with cross-compilation.

---

## Initial Setup

### Install Prerequisites (Ubuntu/WSL)

```bash
sudo apt update
sudo apt install git bc bison flex libssl-dev make crossbuild-essential-armhf gcc-aarch64-linux-gnu
```

### Clone Kernel Source

```bash
cd ~
git clone --depth=1 https://github.com/raspberrypi/linux
cd linux
```


### Configure Kernel

```bash
cd ~/linux
KERNEL=kernel8
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- bcm2711_defconfig
```

### Modify Kernel Version String

```bash
nano .config
# Find: CONFIG_LOCALVERSION="-v8"
# Change to: CONFIG_LOCALVERSION="-v8-JOH"  (use your initials)
```

---

## Git Workflow

### Check Status

```bash
git status
```

### View Changes

```bash
git diff
git diff --cached  # View staged changes
```

### Create Branch

```bash
git checkout -b feature/branch-name
```

### Switch Branch

```bash
git checkout main
git checkout feature/branch-name
```

### Stage Files

```bash
git add filename.c
git add -f filename.c  # Force add if ignored
```

### Commit Changes

```bash
git commit -m "Descriptive commit message"
```

### Commit All Modified Files

```bash
git commit -am "Message"
```

### Push to GitHub

```bash
# First time
git remote add origin https://github.com/username/repo.git
git branch -M main
git push -u origin main

# Subsequent pushes
git push origin branch-name
```

### Pull Changes

```bash
git pull origin main
```

### Merge Branch

```bash
git checkout main
git merge feature/branch-name
```

### Delete Branch

```bash
git branch -d feature/branch-name
git push origin --delete feature/branch-name  # Delete remote
```

---

## Adding Syscalls

### Step 1: Add Function Declaration

```bash
nano include/linux/syscalls.h
```

Add before last `#endif`:

```c
asmlinkage long sys_your_syscall(int param);
```

### Step 2: Register Syscall Number

```bash
nano include/uapi/asm-generic/unistd.h
```

Find `#define __NR_syscalls XXX` and:

1. Increase the number by 1
2. Add BEFORE that line:

```c
#define __NR_your_syscall 466
__SYSCALL(__NR_your_syscall, sys_your_syscall)
```

### Step 3: Add Stub

```bash
nano kernel/sys_ni.c
```

Add anywhere:

```c
COND_SYSCALL(your_syscall);
```

### Step 4: Add to ARM64 Syscall Table

```bash
nano arch/arm64/tools/syscall_64.tbl
```

Add at end (use TAB between columns): USE CORRECT SYSCALL NUMBER

```
466     common  your_syscall    sys_your_syscall
```

### Step 5: Implement Syscall

```bash
nano arch/arm64/kernel/your_file.c
```

Example implementation:

```c
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/io.h>
#include <linux/errno.h>

#define GPIO_BASE 0xFE200000
#define GPIO_SIZE 0x1000
// NOTE THE 2 HERE STANDS FOR 2 ARGUMENTS
SYSCALL_DEFINE2(your_syscall, int, param1, int, param2)
{
    void __iomem *gpio_base;
    
    // Validate inputs
    if (param1 < 0) {
        printk(KERN_ERR "your_syscall: Invalid param\n");
        return -EINVAL;
    }
    
    // Map hardware memory
    gpio_base = ioremap(GPIO_BASE, GPIO_SIZE);
    if (!gpio_base)
        return -ENOMEM;
    
    // Do hardware operations
    iowrite32(value, gpio_base + offset);
    
    // Cleanup
    iounmap(gpio_base);
    
    printk(KERN_INFO "your_syscall: Success\n");
    return 0;
}
```

### Step 6: Add to Makefile

```bash
nano arch/arm64/kernel/Makefile
```

Find `obj-y :=` and add your `.o` file:

```makefile
obj-y := ... your_file.o pi/
```

---

## Building the Kernel

### Full Build (First Time or After Header Changes)

```bash
cd ~/linux
make -j$(nproc) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- Image.gz modules dtbs
```


### Partial Build (After Only .c File Changes)

```bash
cd ~/linux

# Compile just your changed file
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- arch/arm64/kernel/your_file.o

# Relink kernel image (1-2 minutes)
make -j$(nproc) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- Image.gz
```

### Verify Syscall Compiled In

```bash
nm vmlinux | grep your_syscall
# Should show: ... sys_your_syscall
```

### Build Modules Staging

```bash
sudo make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- INSTALL_MOD_PATH=./modules_install modules_install
```

### Create Modules Archive

```bash
tar -czf modules.tar.gz -C modules_install/lib/modules .
```

### Check Build Errors

```bash
# If build fails, scroll up to see the actual error
# Common issues:
# - Missing semicolons
# - Typos in function names
# - Wrong #include statements
```

---

## Copying to Pi

### Find Pi IP Address (Windows)

```cmd
arp -a | findstr 192.168.137
```

### Copy Files (Windows CMD)

```cmd
pushd \\wsl$\Ubuntu-24.04\home\mark\linux

scp arch\arm64\boot\Image.gz mark@192.168.137.234:/tmp/kernel_new.gz
scp arch\arm64\boot\dts\broadcom\*.dtb mark@192.168.137.234:/tmp/
scp modules.tar.gz mark@192.168.137.234:/tmp/

popd
```

Replace `192.168.137.234` with your Pi's actual IP.

### If SCP Hangs

Press Ctrl+C and try again. Sometimes first attempt times out.

---

## Installing on Pi

### SSH to Pi

```cmd
ssh mark@192.168.137.234
```

Or:

```cmd
ssh mark@mark.local
```

### Installation Commands

```bash
# Backup old modules
sudo mkdir -p /home/mark/old_modules_backup
sudo mv /lib/modules/* /home/mark/old_modules_backup/ 2>/dev/null

# Extract new modules
sudo mkdir -p /lib/modules
sudo tar -xzf /tmp/modules.tar.gz -C /lib/modules/

# Backup old kernel (optional)
sudo cp /boot/firmware/kernel8-custom.img /boot/firmware/kernel8-custom.img.backup

# Install new kernel
sudo cp /tmp/kernel_new.gz /boot/firmware/kernel8-custom.img

# Install device trees
sudo cp /tmp/*.dtb /boot/firmware/

# Verify boot config points to custom kernel
grep "kernel=" /boot/firmware/config.txt
# Should show: kernel=kernel8-custom.img

# If not set, add it
sudo nano /boot/firmware/config.txt
# Add at end: kernel=kernel8-custom.img

# Reboot to load new kernel
sudo reboot
```

### After Reboot - Verify

```bash
# Check kernel version
uname -r
# Should show: 6.12.70-v8-JOH+ (or your version)

# Check modules directory
ls /lib/modules/
# Should show only one directory matching uname -r

# Check for errors
dmesg | tail -30
```

---

## Testing

### Write Test Program (on Pi)

```bash
nano test_syscall.c
```

Example:

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>

#define __NR_your_syscall 466

int main() {
    int result = syscall(__NR_your_syscall, 42, 100);
    
    if (result < 0) {
        perror("syscall failed");
        return 1;
    }
    
    printf("Syscall returned: %d\n", result);
    system("dmesg | tail -10");
    return 0;
}
```

### Compile Test Program

```bash
gcc -o test test_syscall.c
```

### Run Test

```bash
sudo ./test
```

### Check Kernel Log

```bash
dmesg | tail -20
dmesg | grep your_syscall
```

### View All Kernel Messages

```bash
dmesg
```

### Clear Kernel Log (for clean testing)

```bash
sudo dmesg -C
```

---


### Pi Loses Internet After Reboot

**Disable and re-enable ICS:**

1. Unplug Pi USB cable
2. Windows: Network Connections → Your WiFi → Properties → Sharing
3. Uncheck "Allow other network users..."
4. Click OK
5. Plug Pi back in
6. Re-enable sharing, select Pi adapter
7. Wait 60 seconds

### Can't SSH to Pi

**Find IP again:**

```cmd
arp -a | findstr 192.168.137
```

**Try hostname:**

```cmd
ssh mark@mark.local
```

## Daily Workflow

### Starting Work

```bash
cd ~/linux

# Pull latest changes from team
git fetch origin
git pull origin main

# Check what changed
git log --oneline -5
```

### Making Changes

```bash
# Create feature branch
git checkout -b feature/my-feature

# Edit files
nano arch/arm64/kernel/gpio_control.c

# Test compile frequently
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- arch/arm64/kernel/gpio_control.o

# When done, commit
git add arch/arm64/kernel/gpio_control.c
git commit -m "Add feature X"

# Push to GitHub
git push origin feature/my-feature
```

### Building and Testing

```bash
# Build kernel
make -j$(nproc) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- Image.gz modules dtbs

# Verify symbol
nm vmlinux | grep my_function

# Prepare modules
sudo make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- INSTALL_MOD_PATH=./modules_install modules_install
tar -czf modules.tar.gz -C modules_install/lib/modules .

# Copy to Pi (from Windows CMD)
# [See "Copying to Pi" section]

# Install on Pi
# [See "Installing on Pi" section]

# Test
sudo ./test_program
dmesg | tail -20
```

### Merging to Main (do not do this without consent, just make a pull request)

```bash
# Switch to main
git checkout main

# Merge your feature
git merge feature/my-feature

# Push to GitHub
git push origin main

# Delete feature branch
git branch -d feature/my-feature
git push origin --delete feature/my-feature
```

---

## Quick Command Reference

### Most Used Commands

**Check where you are:**

```bash
pwd
git branch
uname -r  # On Pi
```

**Build kernel (fast):**

```bash
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- arch/arm64/kernel/your_file.o
make -j$(nproc) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- Image.gz
```

**Verify compilation:**

```bash
nm vmlinux | grep your_function
```

**Copy to Pi:**

```cmd
pushd \\wsl$\Ubuntu-24.04\home\mark\linux
scp arch\arm64\boot\Image.gz mark@192.168.137.234:/tmp/kernel_new.gz
scp modules.tar.gz mark@192.168.137.234:/tmp/
popd
```

**Install on Pi:**

```bash
sudo tar -xzf /tmp/modules.tar.gz -C /lib/modules/
sudo cp /tmp/kernel_new.gz /boot/firmware/kernel8-custom.img
sudo reboot
```

**Test:**

```bash
sudo ./test
dmesg | tail
```

---

## File Locations Reference

### On PC (Ubuntu)

```
~/linux/                                    # Kernel source root
├── arch/arm64/kernel/                      # Your syscall implementations
│   ├── gpio_control.c
│   ├── led_control.c
│   └── Makefile                            # Add your .o files here
├── arch/arm64/tools/syscall_64.tbl         # ARM64 syscall table
├── include/linux/syscalls.h                # Syscall declarations
├── include/uapi/asm-generic/unistd.h       # Syscall numbers
├── kernel/sys_ni.c                         # Syscall stubs
├── .config                                 # Kernel configuration
├── vmlinux                                 # Uncompressed kernel (for nm)
└── arch/arm64/boot/Image.gz                # Compressed kernel (to copy)
```

### On Pi

```
/boot/firmware/
├── kernel8-custom.img                      # Your custom kernel
├── config.txt                              # Boot configuration
└── *.dtb                                   # Device tree files

/lib/modules/
└── 6.12.70-v8-JOH-WIL+/                       # Your kernel modules
    ├── kernel/
    └── modules.*

/home/mark/
├── test_gpio                               # Your test programs
└── old_modules_backup/                     # Backup of old modules
```

---

## Kernel Addresses Reference

### Pi Zero 2 W / Pi 4

```c
#define GPIO_BASE       0xFE200000
```
### GPIO Register Offsets

```c
#define GPFSEL0         0x00    // Function select (pins 0-9)
#define GPFSEL1         0x04    // Function select (pins 10-19)
#define GPFSEL2         0x08    // Function select (pins 20-29)
#define GPSET0          0x1C    // Set pins high
#define GPCLR0          0x28    // Set pins low
#define GPLEV0          0x34    // Read pin levels
```

---

## Syscall Numbers Used

```
463 - hellokernel (test syscall)
464 - gpio_configure
465 - gpio_write
466 - gpio_read
```

---

## Common Error Messages and Fixes

|Error|Meaning|Fix|
|---|---|---|
|Function not implemented|Syscall not in kernel|Verify with `nm vmlinux`, rebuild if needed|
|Invalid argument|Bad parameter passed|Check input validation in syscall|
|Cannot allocate memory|`ioremap()` failed|Check GPIO base address|
|Permission denied|Not running as root|Use `sudo`|
|No such file or directory|Wrong path|Check file location|
|Connection refused (SSH)|Pi not accessible|Check ICS, find IP with `arp -a`|

---

## Tips and Best Practices

1. **Always verify before copying:** Run `nm vmlinux | grep your_function` before copying to Pi
2. **Commit often:** Save your work frequently with git
3. **Test incrementally:** Test each syscall before adding the next
4. **Keep backups:** Always backup old modules and kernels
5. **Read kernel log:** `dmesg` is your friend for debugging
6. **Use branches:** One feature per branch makes collaboration easier
7. **Document as you go:** Update comments in your code
8. **Coordinate syscall numbers:** Use a shared document to track which numbers are used

---

## Emergency Recovery

### If Everything Breaks

**Restore Original Kernel:**

1. Remove SD card from Pi
2. Insert into PC
3. Delete or rename `kernel8-custom.img`
4. Edit `config.txt`, remove `kernel=` line
5. Put SD card back in Pi
6. Boot with original kernel

**Start Fresh:**

```bash
# On PC
cd ~/linux
make clean
git checkout .  # Discard all changes
git pull origin main
# Start over from last good commit
```

