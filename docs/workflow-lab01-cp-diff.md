# Workflow học và bàn giao Lab 01: cp, diff

Cập nhật ngày 08/10/2026. Người phụ trách: Nguyễn Thành Bảo. Phân công của nhóm: cp và diff. Tài liệu này ghi lại quy trình đã thực hiện và cách tiếp tục với diff; không thay thế nhật ký vấn đáp.


## Trạng thái chốt phiên 08/10/2026

Phiên vấn đáp đã kết thúc theo yêu cầu. Các mô tả bên dưới ghi lại từng giai đoạn; trạng thái cuối cùng là cp và diff đều đã được push vào nhánh feat/lab01-cp, [PR #1](https://github.com/Hoaiduc195/os-xv6/pull/1) chờ nhóm trưởng review, chưa merge. Commit bàn giao fb0ba0f; bộ test đã được lưu tại tests/lab01 trên nhánh code, không còn chỉ phụ thuộc các file tạm.

Tài liệu công việc cần làm: docs/lab01-cp-diff-handoff.md trên nhánh code. Có hướng dẫn WSL, checklist review/tích hợp/báo cáo và giới hạn kiểm thử. Toàn bộ test cp/diff/readline đã chạy lại thành công trên QEMU macOS. Ubuntu CI đã khởi chạy; xem check PR để biết kết quả mới nhất. Chưa chạy trực tiếp Windows/WSL.

File workflow này cùng nhật ký vấn đáp được commit/push riêng trên docs/van-dap-cp-diff để tự ôn. Không tiếp tục đặt câu hỏi trong phiên này.

## 1. Trao đổi kiến thức

**File duy nhất ghi câu hỏi và câu trả lời:** [van-dap-cp-diff.md](van-dap-cp-diff.md).

**Nhánh:** `docs/van-dap-cp-diff`.

Quy trình mỗi phần học:

1. Đối chiếu [đề chính thức](CQ_Lab01.docx), [hướng dẫn](xv6-lab01-guide.md) và mã nguồn xv6.
2. Giải thích lý thuyết kèm đoạn code cụ thể, đi từng phần nhỏ.
3. Đặt 1–3 câu hỏi rồi đợi người học trả lời.
4. Ghi nguyên ý câu trả lời thực tế, nhận xét phần đúng/sai, ví dụ và đáp án tham khảo riêng biệt.
5. Ghi rõ điều gì đã được người học hiểu, điều gì còn cần hỏi lại.
6. Kiểm tra diff, chỉ stage nhật ký, commit và push nhánh tài liệu theo ủy quyền trong phiên học.

Đã học với cp: argc/argv, file descriptor, read/write theo buffer, EOF, O_TRUNC, kiểm tra cùng inode và xử lý lỗi. Người học còn cần củng cố phân biệt descriptor với số đối số, và phân biệt truncate với kiểm tra cùng file. Không coi việc code đã chạy là bằng chứng người học đã hiểu hết.

Ví dụ kết luận kỹ thuật: file 1.200 byte, buffer 512 byte → read trả 512, 512, 176, 0; mỗi lần chỉ ghi n byte vừa đọc. O_TRUNC xóa nội dung đích cũ; so sánh dev/ino mới là kiểm tra cùng file.

## 2. Mã nguồn chính để gửi nhóm

**File cp:** [user/cp.c](../xv6-labs-2024/user/cp.c).

**Nhánh bàn giao:** `feat/lab01-cp`, tạo từ `origin/main` trong worktree riêng `/tmp/os-xv6-lab01-cp` để không đưa các commit tài liệu vào PR.

**Commit:** `61005c8`.

**PR:** [#1 — Nguyễn Thành Bảo: triển khai cp và tích hợp UPROGS](https://github.com/Hoaiduc195/os-xv6/pull/1). Đã tạo vào main trong phiên trước; chưa thực hiện merge. Trạng thái GitHub hiện tại chưa được kiểm tra lại trong lượt ghi workflow này.

PR chứa đúng hai file:

- `xv6-labs-2024/user/cp.c`: kiểm tra đối số; mở nguồn chỉ đọc; yêu cầu file thường; bảo vệ cùng file/hard link trước truncate; tạo hoặc ghi đè đích; sao chép từng buffer; kiểm tra lỗi; đóng descriptor.
- `xv6-labs-2024/Makefile`: thêm `$U/_cp` vào `UPROGS` để mkfs đưa chương trình vào image.

Trước commit đã soạn description và commit message nêu rõ phần thực hiện của Nguyễn Thành Bảo. PR này mới hoàn thành cp; diff thuộc phân công nhưng chưa được bàn giao. Description nêu test thực tế và giới hạn kiểm thử WSL.

Workspace học vẫn ở nhánh tài liệu: cp.c còn là file untracked tại đây, nhưng cùng nội dung đã được commit và push trên nhánh tính năng. Makefile workspace có cả dòng `_cp` mới và cờ compiler riêng có sẵn `-Wno-error=unused-but-set-variable`. Cờ riêng này không được đưa vào PR.

Quy trình cho diff khi đã hoàn thành và được yêu cầu bàn giao:

1. Kiểm tra status, fetch GitHub; giữ nguyên thay đổi chưa commit.
2. Dùng nhánh tính năng riêng từ main mới nhất, ưu tiên worktree nếu workspace đang có thay đổi.
3. Đưa diff.c và dòng `_diff` trong UPROGS vào nhánh bàn giao; kiểm tra diff để tránh kéo theo cấu hình riêng hoặc nhật ký học.
4. Build/test bản chuẩn bị commit.
5. Soạn description ghi người phụ trách, phạm vi, test đã chạy và giới hạn.
6. Stage đúng file được phép, commit, push, tạo PR vào main cho nhóm trưởng review. Không force-push hoặc tự merge.

## 3. Kiểm thử

Kiểm thử tách khỏi image làm việc: sao chép cây xv6 vào thư mục tạm, tạo fs.img riêng. Không chạy make clean trong workspace và không xóa dữ liệu test của người học.

### Kiểm thử cp đã thực hiện

Build RISC-V với `TOOLPREFIX=riscv64-elf-`, chạy xv6 trong `qemu-system-riscv64` trên macOS. Chương trình cptest chạy cp bằng fork/exec, kiểm tra mã thoát bằng wait, kích thước bằng fstat và đọc lại từng byte. Dữ liệu theo mẫu i % 251 có cả byte 0.

Các ca đã đạt:

- Nguồn 0, 7, 512, 1.024, 1.200, 4.097 byte: đúng nội dung, kích thước, EOF và bảo toàn nguồn.
- Ghi đè file đích dài hơn; nguồn rỗng ghi đè đích có dữ liệu.
- Descriptor nguồn bằng 0.
- Thiếu/thừa đối số; nguồn không tồn tại; đích không mở được.
- Cùng đường dẫn hoặc hard link: báo lỗi và bảo toàn nguồn.
- Nguồn/đích là thư mục: báo lỗi.

Kết quả kết thúc: `ALL CP TESTS PASSED`.

Sau khi tạo nhánh PR, đã build image có `_cp` trong UPROGS và kiểm thử lại: `echo hello > a`, `cp a b`, `cat b`, `cp a a`, `cat a`. Bản sao đúng và nguồn được giữ nguyên.

Build cp riêng thành công với cờ mặc định. Build toàn bộ image bằng GCC 16 gặp cảnh báo có sẵn trong usertests.c về biến fsblocks. Đã dùng cờ bỏ coi cảnh báo này là lỗi trên dòng lệnh để chạy thử image; không sửa Makefile của PR vì vấn đề đó.

### Bằng chứng trên máy

- `/tmp/xv6-cp-qemu.log`: log bộ kiểm thử chi tiết.
- `/tmp/xv6-cp-test-path`: chứa đường dẫn cây test tạm; trong đó có `user/cptest.c` và `test.mk`.
- `/tmp/os-xv6-cp-pr-qemu.log`: log smoke test nhánh PR.
- `/tmp/os-xv6-cp-pr-description.md`: bản mô tả đã chuẩn bị trước commit.

Đây là file tạm, có thể mất khi dọn máy; chưa được lưu vào Git. Nhật ký kiến thức giữ kết luận kiểm thử đã xác nhận.

### Phạm vi chưa xác nhận

Chưa chạy trực tiếp trên Windows/WSL, chưa kiểm thử lỗi đọc giữa chừng, hết dung lượng hoặc thay đổi đường dẫn đồng thời. Không ghi các trường hợp này là đã đạt.

Trên WSL đã cài GNU make, Perl, RISC-V GCC/binutils và QEMU, chạy trong thư mục xv6-labs-2024:

```sh
make TOOLPREFIX=riscv64-linux-gnu- qemu
```

Đây là hướng dẫn dựa trên Makefile; nhóm cần chạy xác nhận trên máy WSL dùng để demo/nộp bài.

## 4. Tiếp tục diff.c

Trạng thái ngày 08/10/2026: diff.c đã hoàn thiện, thêm _diff vào UPROGS, build thành công và 20 ca kiểm thử chương trình trong QEMU đạt (log /tmp/xv6-diff-qemu.log). Chưa commit/push mã nguồn hoặc tạo PR diff. Tiếp tục theo thứ tự hiểu yêu cầu → thiết kế → viết code → kiểm thử → vấn đáp; không đưa toàn bộ lời giải ngay trong lượt bắt đầu.

Yêu cầu đối chiếu đề:

- Hỗ trợ `diff file1 file2`, `diff file1 file2 -q` và dạng ví dụ `diff -q file1 file2`.
- Đọc và so sánh theo dòng cùng thứ tự.
- Giống hệt: chọn không in gì.
- Khác: in dòng bên file1 với `<`, bên file2 với `>`; một bên hết trước thì hiển thị EOF ở bên đó.
- Với -q: chỉ in một lần `diff: files differ` khi phát hiện khác biệt.

Thiết kế từng phần:

1. Phân tích đối số và mở hai file, xử lý lỗi mở file thứ hai.
2. Viết hàm đọc dòng, phân biệt dòng rỗng với EOF, giữ dòng cuối không có newline; chọn cách xử lý dòng dài mà không âm thầm cắt dữ liệu.
3. So sánh, in kết quả và xử lý -q; thống nhất mã thoát vì đề chưa quy định rõ.
4. Tích hợp UPROGS và chạy test trong image riêng; cập nhật nhật ký chỉ theo kết quả thực tế.

Câu hỏi mở đầu (người học đã trả lời đúng, chi tiết trong nhật ký kiến thức): Với `diff -q a b`, argc bằng bao nhiêu, argv nào là tên hai file? Nếu mở a thành công nhưng mở b thất bại, cần đóng descriptor nào?
