# Simple Rosen Zener two level quantum system simulator
https://github.com/user-attachments/assets/f6935e9b-f9e0-40fc-8548-82afc3a7b3cd

## Building
You need cmake and a C++20 capable compiler. Simply execute the following command inside the repo directory:

Windows:
```
cmake -B Build -DCMAKE_BUILD_TYPE=Release && cmake --build Build --config Release -j %NUMBER_OF_PROCESSORS%
```
Linux:
```
cmake -B Build -DCMAKE_BUILD_TYPE=Release && cmake --build Build -j `nproc`
```
