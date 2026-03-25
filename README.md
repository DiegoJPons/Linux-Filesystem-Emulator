### **Linux Filesystem Emulator**

**The Challenge:** Building a low-level data structure in C that emulates the core functionality of a Linux filesystem. This project required a deep understanding of how operating systems represent directory trees and file metadata, necessitating the manual implementation of complex inode and block-level manipulations.

**Technical Architecture & "The Craft":**
* **Virtual File Representation:** Engineered an emulation of the Linux filesystem using custom C structs to represent inodes and data blocks.
* **Manual Memory Management:** Leveraged dynamic memory allocation and pointer arithmetic to manage a virtualized memory space, ensuring all file operations were self-contained and memory-stable.
* **Core File Operations:** Implemented fundamental filesystem behaviors, including file creation, deletion, and directory traversal, by directly manipulating underlying data structures.
* **Inode Manipulation:** Developed logic to handle complex metadata, including permissions, file sizes, and pointers to data blocks, mirroring the behavior of real-world Unix/Linux systems.
* **Systems Rigor:** Validated the implementation through a rigorous suite of unit tests, ensuring the emulator could handle edge cases in file system representation and memory access.

> **Technical Decision Highlight:** "One of the most critical aspects of this project was emulating the way Linux handles nested directory structures. I chose to implement a hierarchical inode-based lookup system that allowed for efficient path resolution. This decision required careful management of pointer-to-pointer relationships in C, but it resulted in a robust architecture that closely resembled the performance and logic of a production filesystem."
