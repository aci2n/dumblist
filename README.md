# dumblist
Web server that only knows how to print `dumbfile` lists (and serve static content).
Mostly an excercise on network programming in C.

## build
```sh
make
```

## dumbfile
A dumbfile is a list of `key=value` pairs separated by newlines.
Keys are generic for the most part.
The only special keys are `title` and `image`, which do what you would expect.
