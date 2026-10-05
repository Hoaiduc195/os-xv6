# Kiến thức cần thiết trước khi làm Lab 01 xv6

Tài liệu này là phần nền tảng cho Lab 01. Mục tiêu là giúp người học hiểu các khái niệm cần dùng trước khi viết chương trình, không cung cấp lời giải hoàn chỉnh cho từng bài.

Lab 01 chủ yếu làm việc ở **user space**. Bạn chưa cần hiểu toàn bộ scheduler, page table hay filesystem kernel; chỉ cần biết cách một chương trình user được biên dịch, đưa vào fs.img, chạy trong shell xv6 và sử dụng các system call có sẵn.

## 1. Sau khi học xong cần làm được gì?

Bạn nên có khả năng:

- Viết một chương trình C chạy trong môi trường xv6.
- Đọc và kiểm tra tham số dòng lệnh qua argc, argv.
- Tạo process bằng fork() và chờ process con bằng wait().
- Giao tiếp giữa các process bằng pipe.
- Đọc, ghi, mở và đóng file bằng file descriptor.
- Duyệt thư mục bằng struct dirent, stat() và đệ quy.
- Thêm chương trình mới vào Makefile, build lại và chạy trong shell xv6.
- Nhận biết các lỗi thường gặp như pipe bị treo, path quá dài hoặc chương trình chưa được đưa vào filesystem.

## 2. Bức tranh tổng thể của một chương trình user

Đường đi của một chương trình Lab 01 thường là:

~~~text
user/hello.c
    ↓ compiler/linker RISC-V
user/_hello
    ↓ mkfs đọc danh sách UPROGS
fs.img
    ↓ QEMU khởi động kernel và filesystem
xv6 shell: $ _hello
~~~

Các vị trí cần biết:

| Vị trí | Vai trò |
|---|---|
| user/*.c | Mã nguồn các chương trình chạy trong xv6 |
| user/user.h | Khai báo system call và hàm thư viện user |
| kernel/fcntl.h | Các cờ dùng với open() |
| kernel/stat.h | Kiểu file và struct stat |
| kernel/fs.h | struct dirent, DIRSIZ và thông tin filesystem |
| Makefile | Biên dịch kernel, user program và tạo fs.img |
| fs.img | Filesystem được QEMU gắn vào xv6 |

Lệnh Linux/Ubuntu như ls, cp, grep chạy trên hệ điều hành thật. Lệnh gõ sau dấu nhắc $ của xv6 chạy bên trong xv6. Hai môi trường này không phải là một.

## 3. Kiến thức C cần có

### 3.1. Hàm main, tham số và mã lỗi

Chương trình user xv6 thường có dạng:

~~~c
#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if(argc != 2){
    fprintf(2, "usage: program argument\n");
    exit(1);
  }

  // Xử lý argv[1].

  exit(0);
}
~~~

argc là số lượng đối số. argv[0] thường là tên chương trình, còn các đối số thật bắt đầu từ argv[1].

Ví dụ khi chạy:

~~~text
$ _cp input.txt output.txt
~~~

thì chương trình nhận:

~~~text
argc = 3
argv[0] = "_cp"
argv[1] = "input.txt"
argv[2] = "output.txt"
~~~

Kiểm tra số lượng đối số trước khi truy cập argv[i]. Khi có lỗi, in thông báo ra file descriptor 2 và gọi exit(1). Khi thành công, gọi exit(0).

### 3.2. Con trỏ, buffer và chuỗi

Các hàm read() và write() làm việc với vùng nhớ thông qua con trỏ:

~~~c
char buf[512];
int n;

while((n = read(fd, buf, sizeof(buf))) > 0){
  if(write(1, buf, n) != n){
    // Xử lý lỗi ghi.
  }
}
~~~

Điểm cần nhớ:

- read() trả về số byte thực sự đọc, không đảm bảo luôn bằng kích thước buffer.
- read() trả về 0 khi gặp EOF.
- Giá trị âm thường biểu thị lỗi.
- Dữ liệu đọc vào không tự động có ký tự kết thúc chuỗi \0.
- Khi xử lý text, phải tự dành chỗ và thêm \0 đúng vị trí.
- Không đọc hoặc ghi vượt quá kích thước buffer.

Các hàm chuỗi cơ bản có sẵn trong user/user.h gồm strlen, strcmp, strcpy, strchr, memmove, memset, memcmp và memcpy. Không giả định rằng mọi hàm của glibc/POSIX đều tồn tại trong xv6.

### 3.3. struct, hằng số và kiểu dữ liệu xv6

Các bài duyệt file thường dùng:

~~~c
struct stat st;
struct dirent de;
~~~

Các kiểu và hằng số này đến từ header của xv6, không phải từ thư viện C của Ubuntu. Đọc đúng header trước khi dùng:

~~~c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "user/user.h"
~~~

### 3.4. Đệ quy

Đệ quy là hàm tự gọi lại chính nó. Một hàm đệ quy hợp lệ cần có:

1. Điều kiện dừng: trường hợp file, thư mục rỗng hoặc không còn dữ liệu.
2. Bước thu nhỏ bài toán: gọi lại với file con, thư mục con hoặc dữ liệu ít hơn.
3. Thu dọn tài nguyên: đóng file descriptor và giải phóng buffer trước khi trả về.

tree, du và mỗi stage của primes đều có thể được thiết kế theo cách này. Thiếu điều kiện dừng sẽ làm chương trình chạy vô hạn hoặc cạn stack/process.

## 4. System call và hàm user cần dùng

Các prototype hiện tại có thể xem trong user/user.h. Những hàm quan trọng của Lab 01:

| Hàm | Ý nghĩa | Điều cần kiểm tra |
|---|---|---|
| sleep(ticks) | Ngủ một số tick | Đối số là số nguyên hợp lệ |
| fork() | Tạo process con | -1 là lỗi; 0 là process con; số dương là PID ở process cha |
| wait(status) | Chờ process con kết thúc | Gọi ở process cha khi cần thu dọn child |
| exit(status) | Kết thúc process | Dùng mã 0 cho thành công, khác 0 cho lỗi |
| pipe(p) | Tạo pipe một chiều | p[0] đọc, p[1] ghi |
| read(fd, buf, n) | Đọc tối đa n byte | 0 là EOF, số âm là lỗi |
| write(fd, buf, n) | Ghi dữ liệu | Kiểm tra số byte đã ghi |
| close(fd) | Đóng file descriptor | Đóng mọi descriptor không còn dùng |
| open(path, flags) | Mở hoặc tạo file | Giá trị âm là lỗi |
| fstat(fd, &st) | Lấy thông tin từ descriptor | Kiểm tra giá trị trả về |
| stat(path, &st) | Lấy thông tin theo path | Hữu ích khi duyệt thư mục |
| exec(path, argv) | Thay thế image của process | Chỉ quay về khi có lỗi |

Các file descriptor mặc định:

~~~text
0: standard input  - stdin
1: standard output - stdout
2: standard error  - stderr
~~~

Vì vậy, printf(...) thường ghi ra descriptor 1, còn fprintf(2, ...) ghi thông báo lỗi ra descriptor 2.

## 5. Process: fork, wait và exit

### 5.1. Giá trị trả về của fork

Sau khi gọi:

~~~c
int pid = fork();
~~~

có ba khả năng:

~~~text
pid < 0  : fork thất bại
pid == 0 : đang chạy trong process con
pid > 0  : đang chạy trong process cha; pid là PID của child
~~~

Hai process tiếp tục từ dòng ngay sau fork(), nhưng mỗi process nhận giá trị trả về khác nhau. Không được giả định process cha hay con luôn chạy trước.

### 5.2. Mẫu điều khiển process

~~~c
int pid = fork();
if(pid < 0){
  // Xử lý lỗi fork.
} else if(pid == 0){
  // Công việc của process con.
  exit(0);
} else {
  // Công việc của process cha.
  wait(0);
}
~~~

wait() giúp process cha chờ child kết thúc và thu dọn trạng thái của child. Nếu tạo process con mà không cần chờ, vẫn phải thiết kế rõ ai chịu trách nhiệm kết thúc và thu dọn.

## 6. File descriptor và pipe

### 6.1. Pipe có hai đầu

~~~c
int p[2];
pipe(p);
~~~

Sau khi tạo thành công:

~~~text
p[0] : đầu đọc
p[1] : đầu ghi
~~~

Một pipe chỉ truyền dữ liệu theo một hướng. Muốn truyền hai chiều giữa cha và con, cần hai pipe.

### 6.2. Đóng đầu pipe không dùng

Mẫu giao tiếp một chiều:

~~~text
process cha: giữ p[1] để ghi, đóng p[0]
process con: giữ p[0] để đọc, đóng p[1]
~~~

Sau khi ghi xong, process ghi phải close(p[1]). read() ở đầu đọc chỉ nhận EOF khi tất cả các descriptor trỏ đến đầu ghi đã được đóng. Nếu quên đóng một bản sao của p[1], process đọc có thể chờ mãi và bài bị treo.

### 6.3. Quy tắc tránh deadlock

- Kiểm tra kết quả của pipe() và fork().
- Đóng đầu pipe không dùng ngay sau fork().
- Đọc trong vòng lặp cho đến khi read() trả về 0.
- Đóng đầu ghi sau khi gửi hết dữ liệu.
- Gọi wait() ở process cha khi child đã hoàn thành nhiệm vụ.
- Không tạo thêm stage khi dữ liệu đầu vào đã hết.

Trong primes, mỗi stage thường đọc số đầu tiên làm prime hiện tại, lọc các số còn lại rồi gửi số chưa bị loại sang stage kế tiếp. Stage mới phải kết thúc khi pipe đầu vào trả về EOF.

## 7. File và filesystem

### 7.1. Mở file

Các cờ thường dùng được khai báo trong kernel/fcntl.h:

~~~c
O_RDONLY  // chỉ đọc
O_WRONLY  // chỉ ghi
O_RDWR    // đọc và ghi
O_CREATE  // tạo nếu chưa tồn tại
~~~

Ví dụ về ý tưởng mở file để đọc:

~~~c
int fd = open(path, O_RDONLY);
if(fd < 0){
  fprintf(2, "cannot open %s\n", path);
  exit(1);
}
~~~

Luôn đóng descriptor sau khi dùng xong:

~~~c
close(fd);
~~~

### 7.2. Vòng lặp đọc file

Không biết trước file có bao nhiêu byte thì đọc theo vòng lặp:

~~~text
while read được số byte > 0:
    xử lý số byte vừa đọc
nếu read < 0:
    báo lỗi
đóng file
~~~

Đừng xử lý toàn bộ nội dung bằng cách giả định file luôn nhỏ hơn một buffer cố định. cp phải sao chép được file lớn hơn buffer; diff phải xử lý dữ liệu được chia thành nhiều lần read().

### 7.3. Thông tin file

struct stat chứa thông tin như loại file, inode và kích thước. Các loại thường gặp:

~~~text
T_FILE    : file thường
T_DIR     : thư mục
T_DEVICE  : thiết bị
~~~

fstat(fd, &st) dùng một descriptor đã mở. stat(path, &st) dùng trực tiếp đường dẫn.

### 7.4. Đọc thư mục

Một thư mục có thể được mở bằng open() rồi đọc từng struct dirent:

~~~text
fd = open(path)
fstat(fd, &st)
nếu st.type là T_DIR:
    while read(fd, &de, sizeof(de)) == sizeof(de):
        bỏ qua entry rỗng
        ghép path thư mục + tên entry
        xử lý path con
close(fd)
~~~

Khi duyệt đệ quy, luôn bỏ qua hai entry đặc biệt:

~~~text
.
..
~~~

Nếu không bỏ qua chúng, chương trình sẽ quay lại thư mục cha hoặc chính nó và đệ quy vô hạn.

### 7.5. Ghép path an toàn

Khi tạo path con, cần dành chỗ cho:

1. path hiện tại;
2. dấu /;
3. tên entry;
4. ký tự kết thúc chuỗi \0.

Trước khi ghi vào buffer, kiểm tra tổng độ dài không vượt quá kích thước buffer. Không dùng con trỏ trỏ vào vùng nhớ đã hết phạm vi hoặc tiếp tục ghi sau cuối mảng.

## 8. So sánh dữ liệu và xử lý dòng

diff cần phân biệt hai mức xử lý:

- So sánh byte: phù hợp khi chỉ cần biết dữ liệu khác nhau.
- So sánh dòng: cần buffer dòng, tìm \n, so sánh từng dòng và xử lý trường hợp dòng dài hơn buffer.

xv6 không cung cấp đầy đủ các hàm tiện ích của glibc như fgets() hoặc getline(). Nếu cần xử lý theo dòng, hãy viết một helper nhỏ dùng read() từng byte hoặc từng block, tự quản lý buffer và điều kiện kết thúc.

## 9. Kiến thức tương ứng với từng bài

| Bài | Ý tưởng cần nắm | Câu hỏi tự kiểm tra |
|---|---|---|
| sleep | Đọc argv, đổi chuỗi thành số, gọi sleep() | Thiếu hoặc thừa đối số xử lý thế nào? |
| pingpong | Cha-con truyền dữ liệu qua pipe | Đầu nào phải đóng ở mỗi process? |
| primes | Pipeline nhiều process và lọc số | Khi nào stage mới dừng? EOF xuất hiện lúc nào? |
| cp | Mở hai file, đọc block và ghi block | Nếu open, read hoặc write lỗi thì sao? |
| tree | Duyệt cây thư mục bằng đệ quy | Làm sao bỏ qua . và ..? |
| du | Tính kích thước theo file/thư mục | Khi gặp thư mục con thì cộng kích thước ở đâu? |
| diff | Đọc và so sánh nội dung file | Nếu một dòng dài hơn buffer thì xử lý thế nào? |

## 10. Đọc source có sẵn thay vì bắt đầu từ số 0

Các chương trình mẫu trong user/ là tài liệu thực hành tốt:

| File | Nên học gì |
|---|---|
| echo.c | argc, argv, in chuỗi |
| cat.c | open, read, write, close và vòng lặp đọc |
| ls.c | fstat, stat, struct dirent, path con |
| wc.c | Đếm dữ liệu khi đọc theo block |
| grep.c | Xử lý chuỗi và dữ liệu theo dòng |
| sh.c | Cách shell dùng fork, exec, wait và pipe |

Khi đọc code mẫu, tập trung vào vòng đời tài nguyên: descriptor nào được mở, process nào sở hữu nó, khi nào đóng và hàm nào chờ child. Không nên sao chép một đoạn code mà chưa hiểu các điều kiện lỗi của nó.

## 11. Thêm và chạy chương trình mới

Giả sử có file user/hello.c.

### 11.1. Thêm vào UPROGS

Trong Makefile, thêm target có dấu gạch dưới:

~~~make
$U/_hello\
~~~

Dấu \ ở cuối dòng cho biết danh sách còn tiếp tục. Đặt target trước dòng kết thúc danh sách.

### 11.2. Build và chạy

Trong Ubuntu:

~~~bash
cd ~/xv6/xv6-labs-2024
make qemu
~~~

Khi thấy shell xv6:

~~~text
$ _hello
~~~

make qemu thường tự build lại file đã thay đổi. Dùng make clean khi cần xóa các artifact cũ hoặc khi filesystem chứa danh sách chương trình cũ; lệnh này không xóa file nguồn .c.

Thoát QEMU bằng Ctrl-A, sau đó nhấn X.

## 12. Kiểm thử theo từng lớp

Đừng chỉ kiểm tra một output đúng. Với mỗi chương trình, nên thử:

1. Case bình thường: đầu vào đúng theo đề.
2. Case thiếu/thừa đối số: chương trình báo usage và thoát có mã lỗi.
3. Case file lỗi: file không tồn tại, không mở được hoặc path không hợp lệ.
4. Case biên: file rỗng, file lớn, thư mục rỗng, dữ liệu chỉ có một phần tử.
5. Case tài nguyên: không bị treo, không để descriptor mở và process cha chờ child đúng cách.

Một checklist ngắn:

- pingpong kết thúc, không đứng chờ.
- primes in đủ prime theo giới hạn của đề và không tạo process vô tận.
- cp sao chép được file lớn hơn buffer.
- tree và du không đi vào . hoặc .. vô hạn.
- diff xử lý được EOF và dòng cuối không có \n.
- Mọi lỗi open, fork, pipe, read, write, stat quan trọng đều được kiểm tra.

## 13. Các lỗi tư duy thường gặp

### Chương trình không xuất hiện trong shell xv6

Kiểm tra ba điểm:

1. File .c nằm trong thư mục user/.
2. Tên target đã được thêm vào UPROGS.
3. Đã build lại để tạo fs.img mới.

### Pipe bị treo

Gần như luôn do một process vẫn giữ đầu ghi hoặc đầu đọc không cần dùng. Vẽ sơ đồ process và đánh dấu descriptor cần giữ/đóng trước khi viết code.

### read() trả về dữ liệu nhưng xử lý sai

read() trả về số byte, không trả về chuỗi C hoàn chỉnh. Hãy dùng số byte trả về để giới hạn dữ liệu được xử lý và tự thêm \0 nếu cần chuỗi.

### Duyệt thư mục vô hạn

Kiểm tra việc bỏ qua . và .., giới hạn path, điều kiện dừng đệ quy và việc đóng descriptor của thư mục.

### Process cha kết thúc quá sớm

Nếu cha cần đảm bảo child hoàn thành, gọi wait(0) hoặc wait(&status) sau khi tạo child. Không dựa vào thứ tự chạy tình cờ của scheduler.

### Dùng nhầm API của Linux

Chương trình chạy trong xv6 phải dùng API được khai báo trong source xv6. Nếu một hàm không có trong user/user.h, không nên tự giả định rằng linker sẽ cung cấp nó.

## 14. Checklist kiến thức trước khi bắt đầu

Bạn có thể bắt đầu code khi tự trả lời được các câu hỏi sau:

- argv[0], argv[1] là gì khi chạy một lệnh có đối số?
- fork() trả về giá trị nào ở cha và con?
- p[0] và p[1] của pipe dùng cho việc gì?
- Khi nào read() trên pipe trả về 0?
- Vì sao phải đóng đầu pipe không dùng?
- read() trả về bao nhiêu loại kết quả?
- File descriptor 0, 1, 2 lần lượt là gì?
- Cách kiểm tra file là file thường hay thư mục?
- Vì sao phải bỏ qua . và ..?
- Sau khi thêm user/foo.c, cần sửa gì để chạy được $ _foo?
- Làm thế nào để kiểm thử cả case thành công và case lỗi?

Nếu còn chưa chắc, hãy mở lại user/cat.c, user/ls.c và user/sh.c, lần theo từng descriptor và từng lần gọi fork()/wait() trước khi viết bài tương ứng.

## Tài liệu liên quan trong repository

- docs/xv6-lab01-guide.md — hướng dẫn thực hiện từng bài và quy trình build/test.
- docs/xv6-instruction.pdf — yêu cầu môn học và các khái niệm xv6 liên quan.
- docs/CQ_Lab01.docx — danh sách bài, output mẫu và tiêu chí chấm.
- docs/book-xv6-riscv-rev4.pdf — mô tả sâu hơn về process, file descriptor, pipe và filesystem.

