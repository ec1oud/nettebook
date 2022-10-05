Assuming you built KF6 with kdesrc-build:

```
$ cmake -DCMAKE_PREFIX_PATH=/home/myuser/src/kde/usr .
```
...because cmake looks at CMAKE_PREFIX_PATH entries + `lib/cmake` or `lib64/cmake`

https://cmake.org/cmake/help/latest/variable/CMAKE_PREFIX_PATH.html

https://cmake.org/cmake/help/latest/command/find_package.html

So then it's built.  Next problem is runtime loading, without actually installing the libs:

```
$ export LD_LIBRARY_PATH=/home/myuser/src/kde/usr/lib
$ cd path/to/nettebook/src
$ ln -s ~/src/kde/usr/lib/plugins/kf6 .
```

