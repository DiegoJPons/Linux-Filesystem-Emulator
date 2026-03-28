# Linux Filesystem Emulator
A C implementation of a simplified Linux-style filesystem: custom structs model **inodes** and **data blocks**, and core operations (create/delete files, directory traversal, metadata) work by manipulating those structures in a self-contained virtual address space rather than calling the real kernel.
Path handling follows a **hierarchical inode-based lookup** so nested directories resolve like Unix: pointer-heavy data (including pointer-to-pointer relationships) keeps the tree navigable while staying close to how real VFS layers think about names and inodes. Dynamic allocation and careful bookkeeping keep the emulator memory-stable across operations. Behavior is validated with a **unit test suite** aimed at edge cases in structure layout and access patterns.
## Features
- **Virtual on-disk model** — Inodes and blocks represented explicitly in C, with metadata such as permissions, sizes, and pointers to data blocks.
- **Memory** — Heap-allocated virtual “disk” / structures with disciplined use of pointers and arithmetic so file operations stay consistent and leak-free in normal use.
- **Core operations** — Create and delete files, walk directories, and perform basic filesystem behaviors by updating the underlying graph of inodes and blocks.
- **Path resolution** — Nested directories resolved through inode-linked lookup rather than ad hoc string hacks, mirroring Unix-style hierarchical organization.
- **Testing** — Unit tests covering representative and edge cases for representation errors and memory-related mistakes.
## Tech stack
| Area | Technologies |
|------|----------------|
| Core | C, structs, dynamic allocation, pointer arithmetic |
| Model | Inodes, data blocks, directory tree, Unix-like metadata |
| Testing | Unit test suite (framework per your course setup) |
## Project structure
Adjust this block to match your repo when you have it on disk:
```
src/            Filesystem logic (inodes, blocks, operations)
include/        Headers (if separated)
tests/          Unit tests
```
Build with your course toolchain (e.g. **GCC** and **Make** or the Makefile you were given). Follow the assignment’s build and run instructions if they differ.
## Note
This was a school project: the implementation is mine, but the repo began from course starter code.
