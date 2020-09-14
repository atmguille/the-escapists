# The Escapists

This game was developed for the PPROG (Programming Project) course at UAM by [Guillermo García Cobo](https://github.com/atmguille), Jesús Morato Martín and me. Since the game should be terminal based we decided to print spaces as if they were pixels. 

The aim of the game is to escape from a high security prison. To do so, you can move using the arrow keys, collect objects and talk to other inmates.

![](/Screenshots/Welcome%20Screen.png)

## How to run the game

This game is meant to run on UNIX machines, since it uses the termios API. To play sounds you will need `afplay` on macOS or `aplay` on Linux, which are usually installed by default. 

First of all we must compile the game.

```bash
make
```

Then, we have to make the terminal's font as small as possible. 

 Finally, we can run the game running the following command.s

```bash
./main
```

