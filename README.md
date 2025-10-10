# SHAirDrop

![I fucking hate AI slop](./github/neckbeard.png) 

It's like AirDrop, except you can only share small images, it's written in C, needs you to use a command line, will spuriously break and catch fire in unexpected circumstances (as a feature), and honestly just kind of hates you.

"ShairDrop" = Shit AirDrop or Shell AirDrop or Share Drop... I don't fucking know make it what you want. Who am I to tell you what to do?

## VERY GOOD DESIGN DOCUMENT

### Avatar Packet

- Width - 1 byte; 8 bits (max. 2^8 - 1 size; 255); boundscheck this!
- Height - 1 byte as well
- Channel count - 1 byte, can only be 1 (grayscale), 3 (RGB), or 4 (RGBA)
   - We will convert all lower chan. counts into RGBA!
- Pixel data itself
   - Will need to manip. this for the above effect
- Also very important is the size
   - Width  height  channel count
- And how big the packet is
   - 1 + 1 + 1 + (width  height  channel count)
- After further thinking, an opcode header makes a lot of sense as well
 - We might get a packet and a size but we have no fucking idea what the packet is meant for... How do we know what to do with it?
 - Do this later
 - I am hungry
 - :D
> Side note: Can't we just pack a struct, reinterpreting it as a bunch of bytes? Why not do this? This seems really a lot easier...

### Pipeline

 1. Load image using Raylib
 2. Extract packet contents
 3. Assemble packet
 4. Send
 5. Server receives image
 6. Server displays image
   - We never save it to filesystem though

#### New Task: Threading!

Right now the draw loop only begins once we've got an image

I want to always draw and only display the image once we receive it though!

For this we will need threading...

...Yay?

- Set up server with window; they are coupled 
- Handle receiving stuff separately (network thread)
    - `sockfd` is created in main thread, not updated critically 
    - `clientsockfd` is created in net thread, requires locking
    - Image & image texture as well
    - The image will be static; will use a flag that says whether it's inited

```
critical:
    Image img;
    Texture2D imageTexture;
    int clientsockfd 
    bool imgInited;
```

> Stretch: Make it so we can update the image at any time via another request; dirty mod

---

Network thread is crashing b/c it tries to update Raylib stuff, but has no notion 

### Notes

- Using TCP INET4 for simplicity
- sendall, recvall to ensure shit makes it across
- No network thread yet; will add tho
- `htonl/ntohl` = `uint32_t` (4 byte)
- `htons/ntohs` = `uint16_t` (32 byte)
    - Since `uint8_t` is a single byte endianness (byte order) doesn't apply
    - Other types of int are simply not supported :(
- Using any graphics functions in Raylib requires `InitWindow()` because this fires up the OGL context
- Even though threads share process state, OpenGL restricts its use to the thread that started it
> i.e., Treat OpenGL calls in a thread as if the thread was a fork 

### Code Style

- PascalCase for functions and structs
- camelCase for variable names
- .clangd does everything else :)
- Prepend all critical shared state with x; e.g. `imgTexture` would be `xImgTexture`
