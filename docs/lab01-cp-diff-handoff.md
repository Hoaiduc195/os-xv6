# Bàn giao Lab 01 — cp và diff

Người phụ trách: Nguyễn Thành Bảo. Phạm vi PR: hai bài cp, diff theo đề CQ_Lab01 và hướng dẫn Lab 01. Không tuyên bố hoàn thành các bài của hai thành viên khác.

## Mã nguồn

- `xv6-labs-2024/user/cp.c`: sao chép theo buffer 512 byte, ghi đè, kiểm tra lỗi và bảo vệ nguồn khi hai đường dẫn cùng inode.
- `xv6-labs-2024/user/diff.c`: so sánh từng dòng cùng thứ tự, hỗ trợ `-q` ở đầu/cuối, dòng rỗng, dòng dài, EOF lệch nhau, dòng cuối không newline và byte 0.
- Makefile đưa `_cp` và `_diff` vào UPROGS. Không kèm cờ compiler riêng trên máy macOS.

Mã thoát cp: 0 thành công, 1 lỗi. Mã thoát diff: 0 giống, 1 khác, 2 lỗi. Đề chưa quy định mã thoát diff, đây là quy ước triển khai để nhóm review. Dòng không có newline có thêm nhãn `\ No newline at end of file` khi in khác biệt. File có tên bắt đầu bằng dấu - dùng dạng `./name`.

## Chạy trên WSL Ubuntu

Windows chưa có WSL: làm theo [hướng dẫn Microsoft](https://learn.microsoft.com/en-us/windows/wsl/install), chạy `wsl --install` trong PowerShell với quyền quản trị và khởi động lại khi được yêu cầu. Các lệnh dưới đây chạy trong terminal Ubuntu của WSL, không chạy trong PowerShell.

```sh
sudo apt-get update
sudo apt-get install -y build-essential perl python3 git gcc-riscv64-linux-gnu binutils-riscv64-linux-gnu qemu-system-misc
```

Checkout nhánh `feat/lab01-cp` trong một bản clone riêng nếu đang có thay đổi chưa commit. Từ thư mục gốc repository:

```sh
python3 tests/lab01/run.py
```

Runner chỉ dùng thư viện chuẩn Python, tạo bản sao tạm, build rồi chạy QEMU. Nó không sửa Makefile hoặc fs.img đang làm việc. Log mặc định `/tmp/xv6-lab01-qemu.log`. Thành công phải có cả ba dòng:

```text
ALL CP TESTS PASSED
ALL DIFF TESTS PASSED
ALL LINE TESTS PASSED
```

Để demo tương tác:

```sh
cd xv6-labs-2024
make TOOLPREFIX=riscv64-linux-gnu- qemu
```

Trong xv6:

```text
echo hello > a
cp a b
diff a b
echo changed > b
diff a b
diff -q a b
cp a a
cat a
```

Thoát QEMU: Ctrl-a rồi x. Lệnh build có thể tạo lại image khi chương trình thay đổi; hãy sao lưu dữ liệu cần giữ trong image trước khi build để demo.

## Kiểm thử và mức xác nhận

Bộ test được lưu bền vững tại `tests/lab01/`:

- cptest.c: file 0/7/512/1024/1200/4097 byte, nội dung nhị phân, ghi đè và truncate, descriptor 0, đối số sai, cùng file/hard link, thư mục và lỗi mở file. Kiểm tra nội dung, kích thước và mã thoát.
- difftest.c: 20 ca, thu output và kiểm tra chính xác từng byte cùng mã thoát; gồm output mẫu, hai vị trí -q, newline cuối, EOF lệch nhau, dòng dài và byte 0.
- linetest.c: 9 kiểm tra hàm đọc dòng, gồm lỗi read với descriptor -1.
- run.py: build/test trong image tạm, thất bại nếu thiếu marker thành công, có FAIL hoặc quá thời gian.

Đã chạy thành công cả ba bộ với RISC-V GCC 16 và QEMU trên macOS. Do cảnh báo có sẵn trong usertests.c ở compiler này, chạy bằng:

```sh
python3 tests/lab01/run.py --toolprefix riscv64-elf- --extra-cflags=-Wno-error=unused-but-set-variable
```

Đã thêm GitHub Actions Ubuntu 24.04 để build/test bằng gcc-riscv64-linux-gnu, không có cờ bỏ lỗi cảnh báo ở trên. Kết quả xem check của PR; việc có workflow không tự có nghĩa test đã đạt.

Chưa chạy trực tiếp trên máy Windows/WSL. Kiểm thử Ubuntu Linux CI, nếu đạt, xác nhận toolchain Linux và QEMU; không thay thế kiểm thử trên máy WSL dùng để nộp bài.

## Công việc nhóm trưởng cần làm

- [ ] Review PR, quy ước mã thoát và định dạng báo thiếu newline cuối.
- [ ] Kiểm tra check Ubuntu CI, chạy lại `python3 tests/lab01/run.py` trên WSL dùng demo và lưu log.
- [ ] Tích hợp với phần các bạn khác, review rồi merge vào main.
- [ ] Hoàn thiện báo cáo và các file nộp (PDF, patch, source zip) đúng tên sinh viên theo đề; tạo gói nộp từ bản sao riêng, tránh make clean trên workspace chứa dữ liệu cần giữ.

Giới hạn chưa kiểm thử: hết dung lượng, lỗi malloc và thay đổi đường dẫn đồng thời. cp kiểm tra cùng inode trước mở đích nhưng không chống race giữa hai thao tác. diff đọc từng byte; ưu tiên dễ hiểu, không phải thuật toán diff tối ưu. printf xv6 không trả trạng thái nên chưa bắt đầy đủ lỗi ghi ở nhãn/thông báo.

## Tài liệu tự ôn riêng

Nhật ký `docs/van-dap-cp-diff.md` và workflow học `docs/workflow-lab01-cp-diff.md` nằm trên nhánh `docs/van-dap-cp-diff`, không đưa vào PR mã nguồn này. Phiên vấn đáp đã kết thúc; các câu chưa trả lời được giữ để tự ôn.
