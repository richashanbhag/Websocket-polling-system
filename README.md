# 🗳️ WebSocket-Based Live Polling System (C)

## 📌 Overview

This project implements a **secure, real-time web-based polling system** using **TCP sockets and WebSocket protocol in C**. Multiple users can authenticate, vote, and receive real-time poll results.

---

## 🚀 Features

* 🔗 WebSocket handshake (SHA1 + Base64)
* 👥 Concurrent clients using **multithreading (pthreads)**
* 🔐 User authentication (file-based)
* 🗳️ Voting system with duplicate vote prevention
* 📊 Real-time result updates
* 🗂️ Persistent storage using files
* 🧾 Event logging (user, IP, timestamp)
* ⚙️ Socket options:

  * SO_REUSEADDR
  * TCP_NODELAY
  * SO_KEEPALIVE

---

## 🛠️ Technologies Used

* C Programming
* TCP Socket Programming
* WebSocket Protocol
* OpenSSL (SHA1, Base64)
* Pthreads (Concurrency)
* HTML + JavaScript (Frontend)

---

## ▶️ How to Run

### 1. Compile Server

```bash
gcc server.c websocket.c database.c logger.c -o server -lpthread -lssl -lcrypto
```

### 2. Run Server

```bash
./server
```

### 3. Open Frontend

Open `index.html` in browser

---

## 👤 Sample Users

```
alice 123
bob 456
jack 777
```

---

## 🔄 Workflow

1. User logs in
2. Server authenticates user
3. User submits vote
4. Vote stored in database
5. Results calculated and sent back
6. Logs updated

---
