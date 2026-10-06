# Ghi chú chạy xv6 trên macOS

## Môi trường đã kiểm tra ngày 06/10/2026

- Mac Apple Silicon (`arm64`), macOS 26.6.2.
- Homebrew tại `/opt/homebrew`.
- Compiler: `riscv64-elf-gcc` 16.2.0.
- QEMU: `qemu-system-riscv64` 11.1.2.
- Source nằm trong `xv6-labs-2024/`, cấu hình `LAB=util`.

Đây là phiên bản thực tế trên máy lúc kiểm tra, không phải yêu cầu phiên bản tối thiểu.

## 1. Kiểm tra công cụ

Chạy trong Terminal macOS:

```sh
xcode-select -p
command -v make riscv64-elf-gcc riscv64-elf-objdump qemu-system-riscv64
riscv64-elf-gcc --version
qemu-system-riscv64 --version
```

Máy hiện tại có các package Homebrew `riscv64-elf-gcc`, `riscv64-elf-binutils` và `qemu`. Không dùng các lệnh `apt` trong `fix-qemu.txt` trên macOS.

Nếu đã cài công cụ nhưng Terminal chưa tìm thấy, kiểm tra PATH. Trên máy Apple Silicon này:

```sh
export PATH="/opt/homebrew/bin:$PATH"
```

Makefile tự dò một số prefix như `riscv64-unknown-elf-`, nhưng chưa dò `riscv64-elf-`. Vì vậy cần truyền `TOOLPREFIX=riscv64-elf-` trong các lệnh dưới đây.

## 2. Build và chạy

Mở Terminal ở thư mục gốc `os-xv6`, rồi chạy:

```sh
cd xv6-labs-2024
make TOOLPREFIX=riscv64-elf- XCFLAGS=-Wno-error=unused-but-set-variable kernel/kernel fs.img
make TOOLPREFIX=riscv64-elf- XCFLAGS=-Wno-error=unused-but-set-variable qemu
```

Target `kernel/kernel fs.img` build cả kernel và filesystem chứa các chương trình user. `make qemu` cũng tự build phần còn thiếu trước khi chạy.

Với GCC trên máy này, cờ `-Wno-error=unused-but-set-variable` cho phép tiếp tục build khi gặp cảnh báo biến được gán nhưng không dùng. Cảnh báo vẫn hiển thị; các lỗi khác vẫn được kiểm tra bởi `-Werror`. Truyền qua `XCFLAGS` để dùng được với Makefile đã commit, không cần phụ thuộc dòng cờ bổ sung đang sửa cục bộ.

Khi khởi động thành công:

```text
xv6 kernel is booting
...
init: starting sh
$
```

Thử trong shell xv6:

```sh
echo macOS-ok
ls
```

Thoát QEMU: giữ **Control**, nhấn **a**, thả ra rồi nhấn **x**. Không dùng Command thay cho Control.

Khi cần build lại từ đầu, thoát mọi phiên QEMU của workspace trước, rồi chạy:

```sh
make TOOLPREFIX=riscv64-elf- clean
make TOOLPREFIX=riscv64-elf- XCFLAGS=-Wno-error=unused-but-set-variable qemu
```

`make clean` xóa cả `fs.img`; dữ liệu tạo trong shell xv6 sẽ mất khi filesystem được tạo lại.

## 3. Chạy bằng VS Code

Mở thư mục gốc `os-xv6` làm workspace. File VS Code sử dụng là `.vscode/tasks.json` (có chữ **s**). `.vscode/task.json` là bản sao theo tên yêu cầu ban đầu; VS Code không tự đọc tên này. Khi chỉnh task, sửa `tasks.json`.

Các task cục bộ đã tạo:

- `xv6: build (macOS)`: build kernel và filesystem; chạy bằng **Command+Shift+B**.
- `xv6: run (macOS)`: mở Command Palette → **Tasks: Run Task** → chọn task để chạy QEMU.
- `xv6: clean (macOS)`: dọn kết quả build sau khi đã thoát QEMU.

Task đặt thư mục chạy là `${workspaceFolder}/xv6-labs-2024` và bổ sung `/opt/homebrew/bin`, `/usr/local/bin` vào PATH. Dùng process task để đường dẫn workspace có dấu hoặc khoảng trắng được xử lý đúng.

Toàn bộ `.vscode/`, gồm cả `task.json` và `tasks.json`, được ignore và không push lên GitHub. Sau khi clone ở máy khác, có thể tạo `.vscode/tasks.json` tối thiểu như sau:

```json
{
  "version": "2.0.0",
  "tasks": [
    {
      "label": "xv6: run (macOS)",
      "type": "process",
      "command": "/usr/bin/make",
      "args": ["TOOLPREFIX=riscv64-elf-", "XCFLAGS=-Wno-error=unused-but-set-variable", "qemu"],
      "options": {
        "cwd": "${workspaceFolder}/xv6-labs-2024",
        "env": { "PATH": "/opt/homebrew/bin:/usr/local/bin:${env:PATH}" }
      },
      "presentation": { "reveal": "always", "focus": true, "panel": "dedicated" },
      "runOptions": { "instanceLimit": 1 },
      "problemMatcher": ["$gcc"]
    }
  ]
}
```

## 4. Lỗi đã gặp và cách xử lý

### Không tìm thấy RISC-V compiler

Không dùng `riscv64-unknown-elf-gcc` nếu máy chỉ cài `riscv64-elf-gcc`. Kiểm tra `command -v riscv64-elf-gcc`, PATH và đối số `TOOLPREFIX`.

### QEMU báo `Failed to get "write" lock`

Thông báo kèm `Is another process using the image [fs.img]?` cho biết không lấy được khóa ghi image. Kiểm tra terminal đang chạy QEMU của workspace này, thoát bằng **Control+a**, rồi **x**, sau đó chạy lại task. Không chạy hai phiên cùng dùng `fs.img`; thoát phiên cũ trước khi clean hoặc tạo lại image.

### Cảnh báo `unused-but-set-variable` trở thành lỗi

Giữ đối số `XCFLAGS=-Wno-error=unused-but-set-variable` như lệnh mẫu. Đây là cách hạ riêng cảnh báo đó xuống warning để chạy source hiện tại với GCC trên máy; khi sửa source nên xem và xử lý nguyên nhân cảnh báo.

## 5. Nhật ký kiểm tra thực tế

Ngày 06/10/2026:

1. Xác nhận compiler và QEMU bằng các lệnh kiểm tra phiên bản.
2. Build lại toàn bộ bằng `make -B TOOLPREFIX=riscv64-elf- XCFLAGS=-Wno-error=unused-but-set-variable kernel/kernel fs.img`: thành công.
3. Chạy `make ... qemu` với image chính: gặp lỗi khóa ghi `fs.img`.
4. Sao chép image đã build sang thư mục tạm và truyền đường dẫn đó qua `QEMUOPTS` để kiểm tra riêng: thấy `init: starting sh` và dấu nhắc `$`.
5. Chạy `echo macOS-ok`: nhận đúng `macOS-ok`; thoát QEMU bằng **Control+a**, rồi **x**, mã thoát 0.

Kết quả xác nhận build và khởi động shell; chưa chạy bộ chấm Lab 01 hoặc toàn bộ `usertests`. Các task được kiểm tra cấu trúc JSON và các lệnh tương ứng trong Terminal, chưa thao tác chạy trực tiếp qua giao diện VS Code.
