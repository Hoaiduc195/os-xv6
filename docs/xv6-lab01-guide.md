# Hướng dẫn thực hiện Lab 01 xv6

Tài liệu này hướng dẫn chuẩn bị môi trường, đọc source, thiết kế bằng mã giả, xây dựng và kiểm thử các chương trình user cho Lab 01. Tài liệu không chứa lời giải C hoàn chỉnh và không phải là report nộp bài.

## 1. Phạm vi bài lab

Các chương trình cần thực hiện:

1. sleep
2. pingpong
3. primes
4. cp
5. tree
6. du
7. diff

Ba bài đầu rèn process, syscall và pipe. Bốn bài sau rèn file descriptor, file system, directory traversal và xử lý chuỗi.

> Tài liệu đề cập sleep trong phần yêu cầu nhưng bảng điểm của đề không liệt kê riêng bài này. Vẫn nên thực hiện và kiểm thử vì đây là bài khởi động để xác nhận môi trường.

## 2. Chuẩn bị môi trường

### 2.1. Cài WSL trên Windows

Mở PowerShell với quyền Administrator:

~~~powershell
wsl --install -d Ubuntu-22.04
~~~

Khởi động lại máy nếu được yêu cầu. Mở Ubuntu lần đầu, tạo user và password.

Kiểm tra trạng thái từ PowerShell:

~~~powershell
wsl --status
~~~

### 2.2. Cài công cụ trong Ubuntu

~~~bash
sudo apt update
sudo apt upgrade
sudo apt install git build-essential gdb-multiarch qemu-system-misc \
  gcc-riscv64-linux-gnu binutils-riscv64-linux-gnu
~~~

Kiểm tra các công cụ chính:

~~~bash
git --version
make --version
qemu-system-riscv64 --version
riscv64-linux-gnu-gcc --version
~~~

Tên compiler có thể khác giữa các bản xv6. Nếu make báo không tìm thấy compiler, mở Makefile để xem giá trị TOOLPREFIX rồi kiểm tra đúng tên executable tương ứng.

### 2.3. Cài trực tiếp trên macOS

macOS không cần WSL. Mở Terminal và cài các công cụ developer:

~~~bash
xcode-select --install
~~~

Cài Homebrew nếu máy chưa có:

~~~bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
~~~

Cài RISC-V toolchain và QEMU:

~~~bash
brew tap riscv/riscv
brew install riscv-tools
brew install qemu
~~~

Trên Mac Intel, Homebrew thường dùng prefix /usr/local; trên Apple Silicon, prefix thường là /opt/homebrew. Thêm thư mục toolchain vào PATH theo prefix thực tế của Homebrew:

~~~bash
echo 'export PATH="$(brew --prefix riscv-gnu-toolchain)/bin:$PATH"' >> ~/.zshrc
source ~/.zshrc
~~~

Kiểm tra:

~~~bash
qemu-system-riscv64 --version
which riscv64-unknown-elf-gcc
which riscv64-linux-gnu-gcc
~~~

Chỉ cần ít nhất một compiler RISC-V xuất hiện và phải khớp với TOOLPREFIX trong Makefile. Nếu Homebrew không cài được toolchain hoặc QEMU trên máy cụ thể, dùng Ubuntu trong máy ảo là phương án dự phòng.

### 2.4. Tải source xv6

Trên Windows/WSL:

~~~bash
cd /mnt/d
mkdir -p HDH
cd HDH
git clone git://g.csail.mit.edu/xv6-labs-2024
cd xv6-labs-2024
~~~

Trên macOS:

~~~bash
mkdir -p ~/HDH
cd ~/HDH
git clone git://g.csail.mit.edu/xv6-labs-2024
cd xv6-labs-2024
~~~

Nếu giao thức git:// không hoạt động, dùng source hoặc URL do giảng viên cung cấp; không tự đổi sang một branch xv6 khác vì tên file và test có thể khác nhau.

### 2.5. Build lần đầu

~~~bash
make clean
make
make qemu
~~~

Kết quả tối thiểu cần thấy:

~~~text
init: starting sh
$
~~~

Thoát QEMU bằng Ctrl-a, sau đó nhấn x.

## 3. Cấu trúc source cần biết

~~~text
xv6-labs-2024/
├── Makefile              # Build kernel, user programs và fs.img
├── kernel/
│   ├── proc.c/proc.h     # Process, fork, wait, exit, scheduler
│   ├── pipe.c            # Pipe trong kernel
│   ├── file.c            # File descriptor
│   ├── fs.c              # File system và inode
│   ├── sysfile.c         # System call thao tác file
│   ├── sysproc.c         # System call process
│   ├── syscall.c         # Dispatch system call
│   ├── syscall.h         # Mã số system call
│   ├── stat.h            # struct stat và loại file
│   ├── fcntl.h           # Cờ open()
│   └── defs.h            # Khai báo hàm kernel
├── user/
│   ├── user.h            # Prototype syscall và thư viện user
│   ├── usys.pl           # Sinh wrapper syscall
│   ├── ulib.c            # Hàm tiện ích user
│   ├── printf.c          # printf của xv6
│   ├── sh.c              # Shell xv6
│   └── *.c               # Các chương trình user
├── mkfs/
│   └── mkfs.c            # Tạo fs.img
└── README
~~~

### 3.1. Đường đi của một chương trình user

~~~text
user/program.c
    ↓ compiler RISC-V và linker
user/_program
    ↓ mkfs đọc danh sách UPROGS
fs.img
    ↓ QEMU nạp kernel và fs.img
xv6 shell: $ program
~~~

Khi thêm chương trình mới:

1. Tạo user/program.c.
2. Thêm dòng tương ứng vào danh sách UPROGS trong Makefile:

~~~make
$U/_program\
~~~

3. Chạy make clean && make.
4. Chạy make qemu và gọi chương trình từ dấu nhắc $.

Không cần thêm system call kernel mới cho các bài trong lab này.

## 4. System call và thư viện cần dùng

| Nhóm | Hàm | Dùng cho |
|---|---|---|
| Process | fork, wait, exit, getpid | pingpong, primes, sleep |
| Timing | sleep | sleep |
| Pipe | pipe, read, write, close | pingpong, primes |
| File | open, read, write, close | cp, diff |
| Directory | fstat, open, read | tree, du |
| String | atoi, strcmp | Phân tích đối số và so sánh dòng |

Một số quy tắc quan trọng:

- fork() trả về 0 trong child, PID của child trong parent và giá trị âm khi lỗi.
- wait() dùng để thu dọn child đã kết thúc.
- read() trả về 0 khi gặp EOF; với pipe, EOF chỉ xuất hiện khi tất cả đầu ghi đã đóng.
- Mọi process phải đóng file descriptor không dùng.
- Luôn kiểm tra giá trị trả về của system call có thể thất bại.
- Khi thao tác directory, bỏ qua entry tên "." và "..".

## 5. Quy trình làm từng bài

Với mỗi bài, thực hiện theo chu trình:

~~~text
Đọc yêu cầu
    → xác định syscall và dữ liệu vào/ra
    → viết mã giả
    → viết chương trình user
    → thêm vào UPROGS
    → make clean && make
    → chạy test thành công và test lỗi
    → kiểm tra không treo, không rò rỉ fd
~~~

### 5.1. sleep

#### Yêu cầu

- Cú pháp: sleep ticks.
- Thiếu đối số phải in thông báo lỗi.
- Đổi chuỗi sang số bằng atoi.
- Gọi system call sleep.
- Thành công thì exit(0), lỗi thì exit(1).

#### Mã giả

~~~text
main(argc, argv):
    nếu argc != 2:
        in usage
        exit(1)

    ticks = atoi(argv[1])
    sleep(ticks)
    exit(0)
~~~

#### Kiểm thử

~~~text
$ sleep 10
$ sleep
$ sleep 0
$ sleep abc
~~~

### 5.2. pingpong

#### Yêu cầu

Cha gửi một byte cho con. Con in:

~~~text
<pid>: received ping
~~~

Sau đó con gửi một byte ngược lại. Cha in:

~~~text
<pid>: received pong
~~~

#### Mã giả

~~~text
main:
    tạo pipe parent_to_child
    tạo pipe child_to_parent
    pid = fork()

    nếu pid < 0:
        báo lỗi và exit(1)

    nếu pid == 0:                         # child
        đóng đầu ghi parent_to_child
        đóng đầu đọc child_to_parent
        đọc một byte từ parent_to_child
        in received ping với getpid()
        ghi một byte vào child_to_parent
        đóng toàn bộ fd còn lại
        exit(0)

    ngược lại:                            # parent
        đóng đầu đọc parent_to_child
        đóng đầu ghi child_to_parent
        ghi một byte vào parent_to_child
        đọc một byte từ child_to_parent
        in received pong với getpid()
        đóng toàn bộ fd còn lại
        wait(0)
        exit(0)
~~~

#### Lỗi thường gặp

- Parent giữ đầu ghi mà child cần đóng, khiến read() không bao giờ nhận EOF.
- Không wait(), tạo zombie.
- Dùng cùng một pipe cho hai chiều và khó kiểm soát hướng truyền.

### 5.3. primes

#### Ý tưởng

Mỗi process trong pipeline giữ một prime riêng:

~~~text
process 1: đọc 2 → in 2 → lọc bội của 2
process 2: đọc số còn lại → in 3 → lọc bội của 3
process 3: đọc số còn lại → in 5 → lọc bội của 5
...
~~~

#### Mã giả

~~~text
main:
    tạo pipe đầu vào
    fork()

    child:
        đóng đầu ghi
        gọi primes(read_end)

    parent:
        đóng đầu đọc
        ghi các số từ 2 đến 280 vào pipe
        đóng đầu ghi để báo EOF
        wait(0)
        exit(0)

primes(read_fd):
    đọc prime đầu tiên từ read_fd
    nếu không đọc được:
        đóng read_fd
        exit(0)

    in prime
    tạo pipe next
    fork()

    child:
        đóng đầu ghi next
        đóng read_fd
        gọi primes(next_read)

    parent:
        đóng đầu đọc next
        lặp khi read(read_fd, number) == sizeof(number):
            nếu number không chia hết cho prime:
                write(next_write, number)
        đóng read_fd và next_write
        wait(0)
        exit(0)
~~~

#### Lỗi thường gặp

- Quên đóng một đầu pipe, khiến process kế tiếp không nhận EOF.
- Parent thoát trước khi pipeline hoàn tất.
- Tạo process vô hạn thay vì dừng khi pipe đầu vào hết dữ liệu.
- Ghi số dưới dạng chuỗi không cần thiết; nên truyền số nguyên 4 byte.

### 5.4. cp

#### Yêu cầu

- Cú pháp: cp src dst.
- Mở source chỉ đọc.
- Tạo hoặc mở destination để ghi đè.
- Sao chép theo buffer, không giả định file vừa một lần read().
- Báo lỗi khi open, read hoặc write thất bại.

#### Mã giả

~~~text
main(argc, argv):
    nếu argc != 3:
        in usage
        exit(1)

    nếu src và dst là cùng file:
        báo lỗi
        exit(1)

    src_fd = open(src, read_only)
    nếu src_fd < 0:
        báo lỗi
        exit(1)

    dst_fd = mở dst để tạo/ghi
    nếu xv6 không hỗ trợ O_TRUNC:
        sau khi kiểm tra src và dst không phải cùng file,
        unlink dst nếu dst đã tồn tại
        mở lại dst với O_CREATE | O_WRONLY

    lặp:
        n = read(src_fd, buffer, buffer_size)
        nếu n == 0:
            dừng
        nếu n < 0:
            báo lỗi đọc
        nếu write(dst_fd, buffer, n) != n:
            báo lỗi ghi

    đóng cả hai fd
    exit(0)
~~~

Kiểm tra kernel/fcntl.h trước khi sử dụng cờ mở file; không tự giả định xv6 có mọi cờ của Linux.

#### Kiểm thử

~~~text
$ echo hello > a
$ cp a b
$ cat b
hello
$ cp a
$ cp nofile out
$ cp a a
~~~

### 5.5. tree

#### Yêu cầu

- Cú pháp cơ bản: tree [path].
- Mặc định path là ".".
- Hỗ trợ -L depth để giới hạn độ sâu.
- Hỗ trợ -d để chỉ in directory.
- Không duyệt "." và "..".
- Chọn một format thụt lề và sử dụng nhất quán.

#### Mã giả

~~~text
main:
    đặt path mặc định là "."
    phân tích path, -L và -d
    gọi tree(path, level=0)

tree(path, level):
    fstat(path)
    in path theo level

    nếu path là file:
        return
    nếu đã đạt max_depth:
        return

    mở directory
    đọc từng dirent
    bỏ qua tên rỗng, "." và ".."
    tạo child_path
    fstat(child_path)

    nếu only_dir và child là file:
        tiếp tục entry kế

    in child_path
    nếu child là directory:
        tree(child_path, level + 1)
~~~

#### Lỗi thường gặp

- Ghép path thiếu dấu / hoặc làm tràn buffer.
- Đệ quy lại chính directory hiện tại.
- In file dù đang bật -d.
- Không dừng đúng khi gặp -L.

### 5.6. du

#### Yêu cầu

- Cú pháp cơ bản: du [path].
- Mặc định chỉ in directory.
- -a in cả file và directory.
- -s chỉ in tổng của path ban đầu.
- Tổng directory là tổng kích thước các entry con.

#### Mã giả

~~~text
main:
    đặt path mặc định là "."
    phân tích -a và -s
    total = du(path)

    nếu summary:
        in total và path

du(path):
    fstat(path)

    nếu là file:
        nếu all_files và không summary:
            in st.size và path
        return st.size

    total = 0
    mở directory
    đọc từng entry
    bỏ qua "." và ".."

    child_total = du(child_path)
    total += child_total

    nếu không summary:
        in child directory và total tương ứng

    return total
~~~

Quyết định format trước khi viết code, ví dụ:

~~~text
<bytes>    <path>
~~~

### 5.7. diff

#### Yêu cầu

- Cú pháp: diff file1 file2 [-q].
- Đọc file theo từng dòng.
- File giống nhau thì không cần in gì.
- File khác thì đánh dấu dòng bằng < và >.
- -q chỉ in diff: files differ khi có khác biệt.
- Xử lý trường hợp một file hết trước file kia.

Format chi tiết nên dùng là:

~~~text
file1:line: < nội dung dòng file1
file2:line: > nội dung dòng file2
~~~

#### Mã giả

~~~text
readline(fd, buffer):
    đọc từng byte
    dừng ở '\n', EOF hoặc khi buffer đầy
    trả về trạng thái đọc và số ký tự

main:
    phân tích file1, file2 và -q
    mở hai file
    line_no = 1

    lặp:
        has1 = readline(file1, line1)
        has2 = readline(file2, line2)

        nếu cả hai đã EOF:
            dừng

        nếu một dòng khác dòng kia:
            nếu quiet:
                in "diff: files differ"
                exit(0)

            in file1:line_no: < line1
            hoặc in file1:line_no: < EOF nếu file1 đã hết
            in file2:line_no: > line2
            hoặc in file2:line_no: > EOF nếu file2 đã hết

        line_no++

    đóng hai file
    exit(0)
~~~

## 6. Build và kiểm thử

Sau khi thêm hoặc sửa chương trình:

~~~bash
make clean
make
make qemu
~~~

Trong shell xv6, chạy các lệnh tương ứng:

~~~text
$ sleep 10
$ pingpong
$ primes
$ cp a b
$ tree .
$ tree . -L 2
$ tree . -d
$ du .
$ du . -a
$ du . -s
$ diff f1 f2
$ diff f1 f2 -q
~~~

Nếu source có target chấm tự động:

~~~bash
make grade
~~~

### Checklist kiểm thử

- Build sạch sau khi thêm từng chương trình.
- Chạy ít nhất một case thành công và một case lỗi cho mỗi chương trình.
- Kiểm tra thiếu/thừa đối số.
- Kiểm tra file không tồn tại và directory rỗng.
- Kiểm tra file lớn hơn buffer đối với cp và diff.
- Kiểm tra pingpong không bị treo.
- Kiểm tra primes in đủ prime đến 280.
- Kiểm tra tree và du không duyệt "." hoặc ".." vô hạn.
- Đóng mọi file descriptor trước khi process kết thúc.
- Gọi wait() ở process cha khi đã tạo child.

## 7. Lỗi thường gặp

### Không tìm thấy compiler

Đọc TOOLPREFIX trong Makefile, sau đó kiểm tra compiler có cùng prefix:

~~~bash
which riscv64-linux-gnu-gcc
which riscv64-unknown-elf-gcc
~~~

### Chương trình không chạy trong shell xv6

Kiểm tra chương trình đã được thêm vào UPROGS chưa, rồi chạy lại:

~~~bash
make clean
make
~~~

### Pipe bị treo

Kiểm tra tất cả process đã đóng đầu đọc/ghi không sử dụng chưa. read() trên pipe chỉ trả EOF khi không còn descriptor nào trỏ đến đầu ghi.

### primes chạy vô hạn hoặc cạn tài nguyên

Kiểm tra:

- Có đóng read_fd và write_fd ở đúng process không.
- Có giới hạn nguồn số ở 280 không.
- Process cha có wait() cho stage kế tiếp không.
- Có tạo stage mới khi đã hết dữ liệu không.

### tree hoặc du đệ quy vô hạn

Luôn bỏ qua entry tên "." và "..", đồng thời ghép path con vào buffer có kích thước giới hạn.

### Lỗi incompatible-pointer-types với rwsbrk

Dấu hiệu:

~~~text
user/usertests.c:...: error: initialization of
void (*)(char *) from incompatible pointer type void (*)(void)
...
{rwsbrk, "rwsbrk" },
~~~

Đây là lỗi compiler trong user/usertests.c. make qemu chưa chạy QEMU ở thời điểm này. Bảng quicktests yêu cầu mỗi test có dạng hàm nhận một tham số char *, nhưng hàm rwsbrk đang được khai báo không có tham số.

Mở user/usertests.c và sửa khai báo:

~~~c
void
rwsbrk(char *s)
{
~~~

Giữ nguyên bảng quicktests và không xóa test. Tham số s có thể không được sử dụng trong thân hàm; điều đó không gây ra lỗi này.

Sau đó build lại:

~~~bash
make clean
make qemu
~~~

Nếu file đã có rwsbrk(char *s) nhưng vẫn gặp lỗi, kiểm tra source có bị trộn giữa hai phiên bản không:

~~~bash
git diff -- user/usertests.c
git status
~~~

docs/fix-qemu.txt không sửa được lỗi này; tài liệu đó chỉ xử lý lỗi QEMU sau khi compiler đã build thành công. Các lệnh apt trong file đó cũng dành cho Ubuntu/WSL, không dùng trực tiếp trên macOS.

### QEMU không khởi động

Kiểm tra:

~~~bash
qemu-system-riscv64 --version
~~~

Nếu QEMU quá cũ hoặc package bị lỗi, tham khảo docs/fix-qemu.txt và hướng dẫn cài đặt chính thức của môn học.

## 8. Tiêu chí hoàn thành

Người thực hiện nên đạt đủ các điểm sau:

- WSL, compiler và QEMU hoạt động.
- Source xv6 build thành công.
- Các chương trình đã được thêm vào UPROGS.
- Mã giả đã được chuyển thành chương trình user tương ứng.
- Các test thành công và test lỗi đều được chạy.
- Không còn chương trình bị treo do pipe hoặc process chưa được thu dọn.
- Output có format nhất quán với yêu cầu đề.
- Đã chạy make clean trước khi đóng gói source.

## Tài liệu tham khảo

- docs/xv6-instruction.pdf — hướng dẫn cài WSL, syscall, process, pipe và page table.
- docs/CQ_Lab01.docx — yêu cầu Lab 01 và output mẫu.
- docs/book-xv6-riscv-rev4.pdf — mô tả interface, process, file descriptor, pipe và file system của xv6.
- docs/fix-qemu.txt — hướng dẫn xử lý QEMU.
- MIT 6.1810 Tools: <https://pdos.csail.mit.edu/6.1810/2024/tools.html>
- MIT xv6 utilities lab: <https://pdos.csail.mit.edu/6.1810/2024/labs/util.html>
