# Network in Windows
Doing the **[CS50 Final Project](https://cs50.harvard.edu/x/2025/project/)**, I decided to go on with C and try something really hard (at least for a beginner as myself at this point): network.
This is NOT my final project, is just the result of some research and tests about network sockets in Windows that I'm planning to use in the real final project.

## Some sources and inspirations
I believe is important to show the material that I used to be able to make this code.

To begin with, I read a little of **[Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)** to understand what is socket and other important terms.
I found the **[Silva's Simple Crossplatform Network C Library](https://github.com/Silva97/canal/tree/master/sockets)** as study material to understand how to make a crossplatform program, but gave up on this idea and chose to focus only in Windows (my current OS).
Then I found the **[Michael](https://michaelg29.github.io/)** videos coding a **[server](https://www.youtube.com/live/QedzfsexcdU?si=WIyf12YV-D4cnfn-)** and a **[client](https://www.youtube.com/live/sqruI_SuNks?si=lChCm0GsKSMo4bU1)** using only the Windows API.

I need to deeply appreciate those people and the others that talked about network online (what also helped me to understand more).

## How to compile
In case you want to compile the code (maybe change something to test things out, what I really recommend to understand a bit more about it, once I don't really trust in my comments), don't forget to attach the Win32 library as "l- ws2_32" (read the .bat files to see how I compiled it).

## PS.:
I tested it in my own machine (by open twice, one as server and one as client) and worked. Also, I tested it with a friend whose machine is connected with mine via LAN (I just send the .exe for him) and also worked. If you test and something goes wrong, or you find something weird or funny, fell free to tell me.
The code support commands implementation (strings that starts with '/'), but I only implemented one (/close, that shutdown the clients and the server socket, ending the connection).
