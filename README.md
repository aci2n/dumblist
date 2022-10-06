# dumblist
Web server that only knows how to print `dumbfile` lists (and serve static content).
Mostly an excercise on network programming in C.

## build
```sh
make
```

## usage
```sh
dumblist [-a bind_address (default: 0)] [-p port (default: 8080)] [-d data_directory (default: ./data)]
```
Default data directory is relative to the executable.
If builiding with `make` and default options, the data directory would be `$root/out/data`.
To get started, you can drop the following `dumbfile` in that directory:
```
Title=Any random house
Year=1927
Price=$500.000
Contact=richguy@mail.com
```
If you drop an `image.jpg` in the data directory and add `Image=image.jpg` to the dumbfile it will display the given image.

## dumbfile
A dumbfile is a list of `key=value` pairs separated by newlines.
Keys are generic for the most part.
The only special keys are `title` and `image`, which do what you would expect.

## screenshot
![sample listing](doc/sample.png "sample listing")
