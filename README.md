# HTPP - Mini HTTP1/1 Webserver

Please write readme .-.

### Building on ubuntu
Tested with GCC-12 cmake-3.27

Depends on standalone asio
```
sudo apt install libasio-dev
cmake -B build -DCMAKE_BUILD_TYPE=Release -DHTPP_SAMPLE_PROJETS=ON
cmake --build build
```

Run example with  
`./build/example/example`