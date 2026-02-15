# Multithreaded HTTP Server in C

A high-performance, concurrent HTTP/1.1 server built from scratch using the **C Sockets API** and **POSIX threads**. This project implements a **Thread Pool** architecture and **Sharded Read-Write Locking** to handle multiple simultaneous client requests with O(1) scheduling overhead.

## 🚀 Key Features
* **Thread Pool Architecture:** Pre-allocates a fixed pool of 20 worker threads to manage incoming connections, preventing system resource exhaustion.
* **Sharded Concurrency:** Implements **127 granular Read-Write locks** (hashed by URI) to allow parallel file reads (GET) while ensuring exclusive access for file writes (PUT).
* **Producer-Consumer Model:** Utilizes a thread-safe task queue synchronized with **Mutexes** and **Condition Variables** to distribute work across the pool.
* **RESTful Operations:** Supports **GET** (Static Asset Delivery) and **PUT** (File Uploads) with proper RFC-compliant header parsing (`Content-Length`).
* **Zero-Dependency:** Custom HTTP parser written in C to process raw TCP streams without external networking libraries.

## 🛠️ Architecture Overview
1.  **Main Thread (Producer):** Listens on port 8080. When a connection is accepted, the socket is pushed to a shared queue.
2.  **Worker Threads (Consumers):** 20 threads sleep on a condition variable. When woken, they dequeue the socket and handle the request.
3.  **Synchronization:**
    * **Queue:** Protected by a global Mutex.
    * **File I/O:** Protected by `pthread_rwlock_t` sharded by filename hash.

## 💻 Getting Started

### Installation
```bash
git clone [https://github.com/jack-dao/multithreaded-httpserver.git](https://github.com/jack-dao/multithreaded-httpserver.git)
cd multithreaded-httpserver
make