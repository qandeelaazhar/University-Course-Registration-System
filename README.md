#  Concurrent Course Registration System
This project is a simulation of a university course registration system where multiple students try to register for limited seats at the same time.
The main goal of this project was to understand how Operating Systems handle concurrency, thread synchronization, and shared resources safely under heavy load.
Instead of building only a command-line project, I combined:
- a multithreaded C backend,
- a Python middleware server,
- and a simple interactive web dashboard
to create a complete full-stack simulation.

#  What This Project Does
Imagine 100 students trying to register for the same course at the exact same moment.
Without proper synchronization:
- seats could become negative,
- data could get corrupted,
- or the system could crash because of race conditions.
This project solves those problems using:
- POSIX Threads (`pthreads`)
- Mutex Locks
- Deadlock Prevention Techniques
- File-Based IPC
- Process Synchronization
  
#  System Architecture
The project is divided into 3 main parts:
##  Frontend Dashboard (`index2.html`)
This is the user interface where you can:
- add students,
- set course capacities,
- assign student priorities,
- and start the simulation visually.

##  Python Middleware (`server1.py`)
The Python server acts as the bridge between the frontend and the C backend.
Its job is to:
- receive data from the dashboard,
- store it safely,
- launch the C program,
- wait for execution to finish,
- and send results back to the frontend.

##  Core Engine (`OS.c`)
This is the heart of the project.
The C program:
- creates concurrent student threads,
- manages course registration,
- prevents race conditions,
- synchronizes shared data,
- and generates real-time logs.

#  Operating System Concepts Used
## 🔹 Multithreading

Each student is represented as an independent thread using:

```c
pthread_create()
```

The main thread waits for all student threads to finish using:

```c
pthread_join()
```

---

## 🔹 Mutex Locks & Race Condition Prevention

To safely manage shared resources, multiple mutexes were implemented:

- `course_mutex`
  - protects course seat updates

- `json_mutex`
  - protects shared logging data

- `stats_mutex`
  - protects success/failure counters

This ensures all critical operations remain atomic and consistent.

---

## 🔹 Deadlock Prevention

The system avoids deadlocks by making sure:
- a thread never holds multiple locks simultaneously,
- locks are released immediately after use,
- and circular waiting conditions never occur.

---

## 🔹 Inter-Process Communication (IPC)

The Python server and C backend communicate using files:

- `web_input.txt`
- `dashboard_data.json`

Python uses:

```python
subprocess.run()
```

which blocks execution until the C process fully finishes.

---

#  Project Structure

```text
├── OS.c
├── server1.py
├── index2.html
├── web_input.txt
└── dashboard_data.json
```

#  Technologies Used

- C
- POSIX Threads (`pthread`)
- Python
- HTML/CSS/JavaScript
- Socket Programming
- File Handling
- Mutex Synchronization
- Multithreading
- JSON
- Inter-Process Communication (IPC)

#  How to Run the Project

## Step 1 — Compile the C Program

```bash
gcc OS.c -o OS -lpthread
```

---

## Step 2 — Run the Python Server

```bash
python3 server1.py
```

---

## Step 3 — Open the Dashboard

Open your browser and visit:

```text
http://localhost:8001/index2.html
```

Now configure the students and courses, run the simulation, and watch the system safely handle concurrent registrations in real time.

---

#  Sample Output

```json
{
  "summary": {
    "total_success": 2,
    "total_failed": 1
  },
  "logs": [
    {
      "timestamp": "12:04:02",
      "name": "Qandeela",
      "priority": "High",
      "course_id": "CS101",
      "status": "SUCCESS"
    },
    {
      "timestamp": "12:04:02",
      "name": "Amman",
      "priority": "Low",
      "course_id": "CS101",
      "status": "FAILED"
    }
  ]
}
```

---

#  What I Learned
This project helped me better understand:
- thread synchronization,
- mutex locking,
- race conditions,
- deadlock prevention,
- IPC,
- backend/frontend communication,
- and how Operating Systems manage concurrency internally.
---

#  Future Improvements
- Database integration
- Better UI/UX
- Real-time graphs & analytics
- Priority scheduling algorithms
- Distributed server support
- Authentication system
