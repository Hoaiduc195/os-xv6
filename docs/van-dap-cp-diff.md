# Vấn đáp cp, diff

Tài liệu ôn tập Lab 01 xv6, bắt đầu ngày 06/10/2026.

## 1. Mục tiêu và tiến độ

Phần phụ trách: **cp và diff**.

Cách học đã thống nhất: hiểu yêu cầu → thiết kế → viết code → kiểm thử → vấn đáp. Mục tiêu là giải thích được từng thao tác, dự đoán kết quả và tự tìm lỗi.

Từ ngày 07/10/2026: giải thích lý thuyết cùng từng đoạn mã cụ thể, đối chiếu mã nguồn xv6; vẫn học từng phần nhỏ và hỏi lại để kiểm tra hiểu bài.

Tài liệu này ghi lại nội dung trao đổi liên quan đến hai bài dưới dạng biên tập, không phải bản chép nguyên văn toàn bộ cuộc trò chuyện. Những câu trả lời tham khảo bổ sung bên dưới không phải câu trả lời của người học.

- Đã đọc yêu cầu và giới thiệu file descriptor, buffer, read().
- Đã đặt câu hỏi về sao chép file 1.200 byte bằng buffer 512 byte.
- Người học đã trả lời; đã ghi nhận phần sửa về EOF và số byte cần ghi. Đang chờ trả lời câu hỏi củng cố bên dưới.
- Đã viết user/cp.c theo yêu cầu mới và build thành công; chưa chạy kiểm thử cp trong QEMU. Chưa triển khai diff.
- Các câu hỏi nâng cao bên dưới là nội dung chuẩn bị cho những buổi tiếp theo.

## 2. Yêu cầu theo đề

### cp

File: `user/cp.c`.

Cú pháp: `cp src dst`.

- Báo lỗi khi thiếu đối số.
- Mở nguồn để đọc; tạo mới hoặc ghi đè đích.
- Sao chép theo buffer, chẳng hạn 512 hoặc 1.024 byte.
- Báo lỗi khi mở, đọc hoặc ghi thất bại.

### diff

File: `user/diff.c`.

Cú pháp đề nêu: `diff file1 file2 [-q]`. Ví dụ của đề còn dùng `diff -q file1 file2`; nên hỗ trợ cả hai vị trí của `-q`.

- So sánh hai file theo từng dòng.
- File giống hệt: chọn không in gì.
- Dòng khác nhau: in nội dung tương ứng với dấu `<` và `>`.
- Khi một file hết trước, xử lý các dòng còn lại của file kia.
- Với `-q`, nếu có khác biệt chỉ in `diff: files differ`.

Ví dụ định dạng chi tiết:

```text
f1:2: < banana
f2:2: > blueberry
f1:4: < EOF
f2:4: > date
```

Đề không quy định rõ mã thoát cho trường hợp hai file khác nhau. Nhóm cần thống nhất và ghi lại lựa chọn; không tự coi quy ước của diff trên Linux là yêu cầu bắt buộc.

## 3. Nội dung đã trao đổi

### Vì sao học cp trước diff?

Cả hai dùng thao tác mở, đọc và đóng file. cp giúp nắm cách làm việc với dữ liệu theo byte; diff bổ sung việc tách dòng, so sánh và xử lý EOF độc lập ở hai file.

### argc và argv của cp a b

```c
argc = 3;
argv[0] = "cp";
argv[1] = "a";
argv[2] = "b";
```

argc tính cả tên chương trình. Kiểm tra số lượng đối số trước khi truy cập argv[1] và argv[2].

### File descriptor là gì?

```c
int fd = open("a", O_RDONLY);
```

fd là số nguyên dùng để tham chiếu đến một file đang mở trong tiến trình. Nó không phải nội dung file hay địa chỉ bộ nhớ của file. open() trả về số âm khi thất bại.

Các descriptor thường có sẵn:

- 0: đầu vào chuẩn.
- 1: đầu ra chuẩn.
- 2: đầu ra lỗi.

### read() hoạt động thế nào?

```c
char buf[512];
int n = read(fd, buf, sizeof(buf));
```

Lệnh yêu cầu đọc tối đa 512 byte:

- n > 0: số byte thực sự đọc được.
- n == 0: EOF, khi đang yêu cầu đọc số byte dương.
- n < 0: lỗi đọc.

read() không tự thêm ký tự kết thúc chuỗi '\0'. Dữ liệu file có thể chứa byte 0, nên cp không dùng strlen(buf) để tính số byte cần ghi.

### Luồng cp dự kiến

1. Kiểm tra đối số.
2. Mở nguồn để đọc.
3. Kiểm tra đích có cùng file với nguồn hay không.
4. Mở hoặc tạo đích để ghi đè.
5. Đọc một phần dữ liệu vào buffer.
6. Ghi đúng phần vừa đọc sang đích.
7. Lặp đến EOF.
8. Đóng các descriptor đã mở và kết thúc.

### Trao đổi ngày 07/10/2026 — read(), EOF và số byte cần ghi

> File nguồn có 1.200 byte, buffer có 512 byte. Giả sử mỗi lần đọc lấy đủ dữ liệu còn có thể lấy, các lần read() trả về những giá trị nào, kể cả lần báo EOF? Vì sao không được luôn ghi 512 byte sau mỗi lần đọc?

**Trả lời thực tế của người học:**

> Câu 1: Lần gọi 1 trả ra 512, lần gọi 2 trả ra 512 và lần 3 trả ra EOF cùng với dừng.
>
> Câu 2: Việc luôn gọi Write làm tăng gấp đôi số lần đọc/ghi vào ổ đĩa ảo QEMU một cách vô ích.

**Nhận xét:** Hai lần đọc đầu đúng. Lần thứ ba vẫn còn 176 byte để đọc, nên chưa trả về 0. Câu 2 chưa đúng: cp cần ghi dữ liệu đã đọc sang file đích. Vấn đề là số byte truyền cho write(), không phải việc gọi write() tự nó vô ích. Đổi đối số từ n thành 512 không tự làm tăng gấp đôi số lần gọi. Số syscall cũng không tương ứng một-một với số thao tác ổ đĩa, vì xv6 có buffer cache và cơ chế log.

<details>
<summary>Đáp án tham khảo và kết luận kỹ thuật</summary>

Các kết quả lần lượt: **512, 512, 176, 0**.

Hai lần đầu chuyển 1.024 byte; lần tiếp theo còn 176 byte. Lần đọc sau đó trả về 0 để báo EOF.

Nếu luôn ghi 512 byte ở lần đọc được 176 byte, chương trình ghi thêm 336 byte không thuộc dữ liệu vừa đọc. Chúng có thể là dữ liệu còn lại trong buffer từ lần trước, khiến file đích sai nội dung và kích thước.

Khi đọc được n byte, cần chuyển đủ n byte đó trước khi đọc tiếp. Không ghi dữ liệu khi n bằng 0 hoặc âm.

</details>

**Đối chiếu mã nguồn thực tế:** `xv6-labs-2024/user/cat.c` dùng đúng mẫu sau (trích đoạn):

```c
while((n = read(fd, buf, sizeof(buf))) > 0) {
  if (write(1, buf, n) != n) {
    fprintf(2, "cat: write error\n");
    exit(1);
  }
}
if(n < 0){
  fprintf(2, "cat: read error\n");
  exit(1);
}
```

Ở cat, descriptor 1 là đầu ra chuẩn. Với cp, ý tưởng tương ứng là `write(dstfd, buf, n)`, với dstfd là descriptor file đích đã mở. Đây chỉ là phần đọc/ghi để học, chưa phải chương trình cp hoàn chỉnh. Điều kiện `> 0` cho phép xử lý cả lần đọc được 176 byte; khi n == 0 thì thoát vòng lặp, còn n < 0 được báo lỗi riêng.

Trong `kernel/file.c`, fileread() tăng vị trí đọc `f->off` thêm số byte thực sự đọc được. Trong `kernel/fs.c`, readi() giới hạn số byte bằng phần còn lại:

```c
if(off + n > ip->size)
  n = ip->size - off;
```

Với file ổn định 1.200 byte, lần ba bắt đầu ở offset 1.024 nên đọc 176 byte. Lần bốn bắt đầu ở offset 1.200 nên đọc 0 byte. EOF không phải một byte đặc biệt được chép vào buffer. `kernel/bio.c` cũng cho thấy bread() chỉ gọi đọc ổ đĩa khi block trong cache chưa hợp lệ.

**Ví dụ lỗi:** Nếu file đích ban đầu rỗng và ba lần ghi đều thành công với 512 byte, đích dài 1.536 byte thay vì 1.200 byte. Ở lần cuối, chỉ `buf[0]` đến `buf[175]` là dữ liệu mới; `buf[176]` đến `buf[511]` còn dữ liệu cũ từ lần đọc trước.

**Kết luận để ghi nhớ:** read() trả về số byte vừa đọc; ghi đúng số byte đó. Đọc được ít hơn kích thước buffer vẫn phải xử lý dữ liệu; với yêu cầu đọc dương, giá trị 0 mới báo EOF. Chưa xác nhận người học đã nắm vững phần sửa, cần trả lời câu hỏi củng cố.

**Kiểm chứng:** Đã đọc mã nguồn cat.c, file.c, fs.c và bio.c. Chưa biên dịch hoặc chạy test cho phần này; các con số trên là phân tích lý thuyết, không phải kết quả test đã đạt.

### Câu hỏi củng cố về read() — tạm để ôn lại

1. Trong đoạn code trên, lần read() trả về 176 thì có vào thân vòng while không? Với cp, lệnh ghi lúc đó phải là gì?
2. Nếu thay điều kiện `> 0` bằng `== sizeof(buf)`, file nguồn 1.200 byte sẽ được sao chép bao nhiêu byte (giả sử mọi lần ghi thành công)? Vì sao?

**Trả lời của người học:** Chưa có.

### Trao đổi ngày 07/10/2026 — tự viết cp: kiểm tra đối số

**Câu hỏi:**

1. Vì sao dùng `argc != 3` thay vì `argc < 3`?
2. Nếu người dùng chỉ gõ `cp a`, vì sao phải kiểm tra đối số trước khi mở `argv[2]`?

**Trả lời thực tế của người học:**

> 1. Vì lệnh copy cần chính xác 3 giá trị đầu vào. 2. kiểm trả tính hợp lệ của a.

**Nhận xét và đáp án tham khảo:**

- Câu 1 đúng về số lượng. Nói chính xác: argc tính cả tên chương trình; `cp a b` có 3 phần tử, gồm tên lệnh và 2 đối số đường dẫn. `argc != 3` từ chối cả thiếu lẫn thừa đối số, còn `argc < 3` vẫn chấp nhận `cp a b c` dù chương trình chưa hỗ trợ cú pháp đó.
- Câu 2 chưa đúng. Với `cp a`, argc bằng 2, argv[1] là "a" nhưng argv[2] là con trỏ null kết thúc danh sách đối số, không phải đường dẫn đích. Kiểm tra argc bảo đảm đủ đối số trước khi sử dụng chúng. Kiểm tra file a có mở để đọc được hay không là bước gọi open() và kiểm tra kết quả sau đó.

**Mã khung đã hướng dẫn, chưa xác nhận được nhập vào file:**

```c
#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if(argc != 3){
    fprintf(2, "Usage: cp src dst\n");
    exit(1);
  }

  // Phần tiếp theo: mở file nguồn.
  exit(0);
}
```

**Kết luận kỹ thuật:** Kiểm tra cú pháp trước, mở file sau. Số lượng đối số hợp lệ chưa chứng minh file nguồn tồn tại hoặc mở được. Chưa xác nhận người học đã hiểu phần sửa câu 2.

**Bước tiếp theo — chèn sau kiểm tra argc:**

```c
int srcfd = open(argv[1], O_RDONLY);
if(srcfd < 0){
  fprintf(2, "cp: cannot open %s\n", argv[1]);
  exit(1);
}

// Tạm đóng nguồn khi chưa viết phần sao chép.
close(srcfd);
```

Đây vẫn là chương trình đang xây dựng, chưa sao chép dữ liệu. O_RDONLY mở nguồn chỉ để đọc. open() trả descriptor không âm khi thành công, giá trị âm khi thất bại; descriptor 0 cũng có thể hợp lệ. Khi viết phần sao chép, chuyển close(srcfd) xuống sau khi dùng xong nguồn.

**Câu hỏi tiếp theo:**

1. Nếu gõ `cp a` và file a thực sự tồn tại, lệnh đã đủ đối số chưa? Thiếu gì?
2. Vì sao kiểm tra `srcfd < 0` thay vì `srcfd <= 0`?

**Trả lời thực tế của người học:**

> 1. chưa đủ, thiếu đích để sao chép. 2. loại trước hợp có đối số.

**Nhận xét:** Câu 1 đúng, đã phân biệt được file nguồn tồn tại với việc đủ đối số. Câu 2 chưa đúng: srcfd không biểu thị số lượng hoặc sự tồn tại của đối số; đó là kết quả open().

**Đáp án tham khảo:** argc là số phần tử của danh sách đối số, còn srcfd là descriptor dùng để truy cập file đã mở. open() trả về số không âm khi thành công và -1 khi thất bại trong xv6. Giá trị 0 cũng là descriptor hợp lệ, nên `srcfd <= 0` sẽ báo lỗi nhầm khi open() thành công với descriptor 0. Khi 0, 1, 2 đang được dùng và 3 còn trống, open() thường nhận descriptor 3; nếu slot 0 đang trống, nó có thể nhận 0.

**Đối chiếu mã nguồn:** `xv6-labs-2024/kernel/sysfile.c`, fdalloc() duyệt từ fd = 0, gán file vào slot trống đầu tiên rồi trả về fd; nếu không còn slot thì trả về -1:

```c
for(fd = 0; fd < NOFILE; fd++){
  if(p->ofile[fd] == 0){
    p->ofile[fd] = f;
    return fd;
  }
}
return -1;
```

**Kết luận:** Đã xác nhận hiểu yêu cầu có đường dẫn đích. Cần củng cố sự khác nhau giữa argc và srcfd trước khi viết tiếp phần mở đích.

**Câu hỏi củng cố đang chờ:** Giả sử open() lần lượt trả về -1, 0 và 3. Trường hợp nào báo lỗi, trường hợp nào mở thành công? Điều kiện `srcfd <= 0` nhận định sai trường hợp nào?

**Trả lời của người học:** Chưa có.

**Kiểm chứng:** user/cp.c đã tồn tại nhưng nội dung trên đĩa còn rỗng lúc kiểm tra. Chưa build hoặc chạy test; các đoạn trên là mã hướng dẫn. Không thay đổi hoặc commit cp.c và Makefile.


### Triển khai cp theo yêu cầu ngày 07/10/2026

**Yêu cầu thực tế của người học:**

> code cp.c dựa trên file tôi đan glafm

File cp.c trên đĩa vẫn rỗng khi kiểm tra. Đã viết chương trình dựa trên khung đã học: kiểm tra argc, mở nguồn O_RDONLY, fstat để xác nhận file thường, kiểm tra đích cùng dev/ino trước khi truncate, mở đích O_CREATE | O_WRONLY | O_TRUNC, sao chép bằng buffer 512 byte, đóng descriptor và trả mã thoát. Đích đã tồn tại cũng phải là file thường. Báo lỗi nếu đọc thất bại hoặc ghi không đủ n byte. Trong kernel/file.c của bản xv6 này, filewrite() với file thường trả n hoặc -1; cách kiểm tra `write(...) != n` phù hợp với hành vi đó.

**Kết quả kiểm tra thực tế:**

- `make user/_cp` thất bại vì Makefile không tự nhận diện prefix toolchain cài trên máy.
- `make TOOLPREFIX=riscv64-elf- user/_cp` thành công, tạo executable user/_cp.
- Chưa chạy test hành vi trong QEMU; chưa đánh dấu trường hợp sao chép nào là đã đạt.
- Giữ nguyên Makefile. Chưa thêm cp vào UPROGS; cần bổ sung khi đến bước chạy trong xv6.
- Chỉ commit nhật ký; cp.c giữ ở working tree để người học tiếp tục làm.

**Giới hạn:** Kiểm tra cùng inode và mở đích là hai thao tác riêng, không bảo vệ trước tiến trình khác đồng thời thay đổi đường dẫn đích. Nếu lỗi xảy ra sau khi truncate hoặc đang ghi, file đích có thể chỉ chứa một phần dữ liệu.

**Câu hỏi đọc code tiếp theo:** Vì sao phải kiểm tra nguồn và đích có cùng inode trước khi mở đích với O_TRUNC?

**Trả lời của người học:** Chưa có.

## 4. Bộ câu hỏi cp — chuẩn bị cho buổi tiếp theo

### Vì sao không đọc cả file vào một buffer cố định?

File có thể lớn hơn buffer. Đọc và ghi từng phần cho phép sao chép file lớn mà chỉ dùng một lượng bộ nhớ nhỏ.

### O_CREATE có tự xóa nội dung cũ không?

Không. Trong phiên bản mã nguồn đã kiểm tra, kernel/fcntl.h có O_TRUNC. Có thể dùng O_CREATE | O_WRONLY | O_TRUNC cho đích, sau khi đã kiểm tra không trùng nguồn.

Nếu không truncate, khi ghi nguồn ngắn hơn đích cũ, phần đuôi cũ có thể còn lại.

### Vì sao phải kiểm tra cp a a trước khi truncate?

Truncate đích cũng làm mất dữ liệu nguồn nếu cả hai cùng chỉ một file. Descriptor nguồn đã mở không giữ lại một bản sao riêng của nội dung.

### So sánh tên đường dẫn đã đủ để phát hiện cùng file chưa?

Chưa. Hai đường dẫn khác nhau có thể chỉ đến cùng inode, chẳng hạn qua hard link. Có thể so sánh dev và ino từ thông tin stat của hai file trong môi trường lab.

### Nếu write() trả về ít hơn số byte yêu cầu?

Phải kiểm tra kết quả. Một cách xử lý là lặp để ghi phần còn lại; kết quả âm hoặc không tiến triển phải được xử lý như lỗi. Không báo thành công nếu chưa ghi đủ dữ liệu.

Cần đối chiếu hành vi thực tế của write() trong xv6 khi triển khai, thay vì giả định mọi chi tiết đều giống Linux.

### Nếu mở nguồn được nhưng mở đích thất bại?

Báo lỗi ra descriptor 2, đóng descriptor nguồn và thoát với mã lỗi.

### Vì sao phải close()?

Tiến trình có giới hạn descriptor. close() giải phóng tham chiếu không còn dùng. xv6 cũng dọn các file đang mở khi exit(), nhưng đóng đúng lúc giúp quản lý tài nguyên rõ ràng.

### Có cần thêm syscall mới cho cp không?

Không. Sử dụng các syscall có sẵn như open, read, write, close và fstat.

## 5. Bộ câu hỏi diff — chuẩn bị cho buổi tiếp theo

### Khác nhau giữa so sánh byte và so sánh dòng?

So sánh byte xác định nội dung có khác nhau không. So sánh dòng còn cần nhận diện dấu xuống dòng và giữ số thứ tự dòng để in vị trí khác biệt.

### EOF có phải ký tự trong file không?

Không. EOF là trạng thái được nhận biết khi read() trả về 0 với yêu cầu đọc số byte dương. Chuỗi "EOF" trong output mẫu chỉ là nhãn chương trình in ra.

### Dòng rỗng có giống EOF không?

Không. Dòng rỗng vẫn có dấu xuống dòng. Hàm đọc dòng cần phân biệt được dòng rỗng với việc không còn dữ liệu.

### Một file hết trước thì làm gì?

Tiếp tục đọc file còn lại. Các dòng còn lại được báo là chỉ có ở file đó; theo định dạng mẫu, phía file đã hết hiển thị EOF.

### Dòng cuối không có '\n' có bị bỏ qua không?

Không. Nếu đã đọc được dữ liệu trước EOF thì vẫn phải xử lý phần dữ liệu đó như dòng cuối.

Cần giữ thông tin có hay không dấu xuống dòng để phân biệt nội dung "abc" với "abc\n" khi kiểm tra giống hệt.

### Dòng dài hơn buffer thì sao?

Không được âm thầm cắt dòng hoặc coi mỗi mảnh buffer là một dòng mới. Cần chọn cách tăng buffer hoặc xử lý dòng theo nhiều phần; nếu đặt giới hạn thì phải báo lỗi rõ ràng khi vượt giới hạn.

### Dùng strcmp() cần điều kiện gì?

Buffer phải có '\0' kết thúc và đủ chỗ cho ký tự đó. strcmp() phù hợp với dòng văn bản; nó không so sánh đầy đủ dữ liệu nhị phân có byte 0 nằm giữa.

### -q có nghĩa là không in gì không?

Không. Theo đề, nếu khác nhau thì in một lần "diff: files differ"; nếu giống nhau thì không in gì. Khi đã xác định khác biệt, có thể kết thúc sớm sau khi đóng các file.

### Bài này có cần thuật toán diff tối ưu không?

Gợi ý của đề là so sánh hai dòng cùng thứ tự. Đây là cách cơ bản, không phải thuật toán tìm chuỗi chỉnh sửa tối thiểu. Chèn một dòng ở giữa có thể khiến nhiều dòng tiếp theo bị báo khác.

### Làm sao tạo dữ liệu nhiều dòng trong xv6?

Không giả định echo hỗ trợ -e: user/echo.c trong bản đã kiểm tra chỉ in đối số và không diễn giải escape. Có thể dùng một chương trình tạo dữ liệu test nhỏ với write(), hoặc cách tạo fixture đã kiểm chứng trong shell xv6.

## 6. Checklist kiểm thử dự kiến

Các trường hợp dưới đây **chưa được chạy trong phiên học**.

### cp

- File nhỏ và file rỗng.
- File 1.200 byte với buffer 512 byte.
- File có kích thước đúng bội số của buffer.
- Ghi đè file đích dài hơn nguồn.
- Thiếu hoặc thừa đối số; nguồn không tồn tại.
- Nguồn và đích cùng đường dẫn.
- Hai đường dẫn là hard link đến cùng file.
- Nguồn là thư mục; đích không mở được.
- Kiểm tra lỗi ghi, chẳng hạn hết dung lượng.
- Đối chiếu cả nội dung lẫn kích thước; không chỉ nhìn thông báo thành công.

### diff

- Hai file giống nhau; cả hai rỗng.
- Khác dòng đầu, giữa hoặc cuối.
- Một file rỗng; một file có thêm dòng.
- Dòng rỗng nằm giữa nội dung.
- Dòng dài hơn buffer.
- Dòng cuối không có newline.
- Cùng nội dung chữ nhưng khác newline ở cuối file.
- Cả hai cú pháp đặt -q.
- Thiếu đối số, option không hợp lệ, file không mở được.

## 7. Ghi chú môi trường và làm việc nhóm

- Đề ghi môi trường Linux. Viết và chạy trên macOS được, nhưng nhóm cần kiểm thử trên Linux và xác nhận yêu cầu demo với giảng viên.
- tasks.json chỉ giúp VS Code gọi lệnh; Makefile quy định cách build.
- Tên executable trên máy host có dạng user/_cp; mkfs bỏ dấu "_" khi đưa vào fs.img, nên trong xv6 gọi cp.
- Khi thêm chương trình, bổ sung $U/_cp và $U/_diff vào UPROGS.
- Chạy make clean sẽ xóa fs.img của bản này; lưu ý dữ liệu test tạo bên trong xv6 có thể mất.
- Không đưa cấu hình compiler riêng của một máy vào cấu hình chung khi nhóm chưa thống nhất.

## 8. Cách bổ sung nhật ký học

Sau mỗi lượt trao đổi, bổ sung:

1. Câu hỏi.
2. Câu trả lời thực tế của người học.
3. Nhận xét và điểm cần sửa.
4. Kết luận đã thống nhất.
5. Mã hoặc test liên quan và kết quả thực tế.

Không ghi một test là "đã đạt" nếu chưa chạy, và không điền đáp án tham khảo thành lời của người học.

## 9. Nguồn đối chiếu

- [Đề Lab 01](CQ_Lab01.docx).
- [Hướng dẫn thực hiện](xv6-lab01-guide.md).
- [Kiến thức nền](xv6-lab01-knowledge.md).
- [Tài liệu xv6](xv6-instruction.pdf).
- Mã nguồn trong xv6-labs-2024: user/user.h, user/echo.c, kernel/fcntl.h và mkfs/mkfs.c.

Ưu tiên đề chính thức và hành vi mã nguồn đang sử dụng khi tài liệu hướng dẫn có điểm khác nhau.
